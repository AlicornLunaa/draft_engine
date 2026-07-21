#include "draft/interface/imgui/profiler_system.hpp"

#include "imgui.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#else
    #include <unistd.h>
#endif

namespace Draft {
    namespace {
        constexpr std::size_t HISTORY_CAPACITY = 120;
        constexpr std::chrono::milliseconds SAMPLE_INTERVAL{500};

        float get_history_value(void* data, int idx){
            return (*static_cast<CircularBuffer<float>*>(data))[idx];
        }

        float get_fps_history_value(void* data, int idx){
            return 1.f / (*static_cast<CircularBuffer<float>*>(data))[idx];
        }

        #if defined(_WIN32)
            std::uint64_t filetime_to_ticks(const FILETIME& ft){
                return (static_cast<std::uint64_t>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
            }

            // FILETIME-based counters are in 100ns units on every Windows API used below.
            double process_ticks_to_seconds(std::uint64_t ticks){
                return static_cast<double>(ticks) / 1e7;
            }

            std::uint64_t read_process_cpu_ticks(){
                FILETIME creation, exit, kernel, user;
                if(!GetProcessTimes(GetCurrentProcess(), &creation, &exit, &kernel, &user))
                    return 0;

                return filetime_to_ticks(kernel) + filetime_to_ticks(user);
            }

            std::uint64_t read_process_memory_bytes(){
                PROCESS_MEMORY_COUNTERS pmc{};
                if(!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
                    return 0;

                return pmc.WorkingSetSize;
            }

            void read_system_cpu_ticks(std::uint64_t& outTotal, std::uint64_t& outBusy){
                FILETIME idle, kernel, user;
                if(!GetSystemTimes(&idle, &kernel, &user)){
                    outTotal = outBusy = 0;
                    return;
                }

                // kernelTime already includes idleTime on Windows.
                std::uint64_t idleTicks = filetime_to_ticks(idle);
                outTotal = filetime_to_ticks(kernel) + filetime_to_ticks(user);
                outBusy = outTotal - idleTicks;
            }

            void read_system_memory_bytes(std::uint64_t& outUsed, std::uint64_t& outTotal){
                MEMORYSTATUSEX status{};
                status.dwLength = sizeof(status);
                if(!GlobalMemoryStatusEx(&status)){
                    outUsed = outTotal = 0;
                    return;
                }

                outTotal = status.ullTotalPhys;
                outUsed = status.ullTotalPhys - status.ullAvailPhys;
            }
        #else
            // /proc/self/stat's 2nd field is "(comm)", which can itself contain spaces/parens, so
            // every field is only safely indexable after the last ')'. utime/stime are fields 14/15
            // overall, i.e. tokens 11/12 counting from the first token after "comm) ".
            std::uint64_t read_process_cpu_ticks(){
                std::ifstream file("/proc/self/stat");
                std::string line;
                if(!file || !std::getline(file, line))
                    return 0;

                std::size_t close = line.rfind(')');
                if(close == std::string::npos || close + 2 > line.size())
                    return 0;

                std::istringstream rest(line.substr(close + 2));
                std::string token;
                std::uint64_t utime = 0, stime = 0;

                for(int field = 0; field < 13 && rest >> token; field++){
                    if(field == 11) utime = std::stoull(token);
                    else if(field == 12) stime = std::stoull(token);
                }

                return utime + stime;
            }

            double process_ticks_to_seconds(std::uint64_t ticks){
                static const double clockTicksPerSecond = static_cast<double>(sysconf(_SC_CLK_TCK));
                return static_cast<double>(ticks) / clockTicksPerSecond;
            }

            std::uint64_t read_process_memory_bytes(){
                std::ifstream file("/proc/self/statm");
                std::uint64_t totalPages = 0, residentPages = 0;
                if(!(file >> totalPages >> residentPages))
                    return 0;

                return residentPages * static_cast<std::uint64_t>(sysconf(_SC_PAGESIZE));
            }

            void read_system_cpu_ticks(std::uint64_t& outTotal, std::uint64_t& outBusy){
                std::ifstream file("/proc/stat");
                std::string cpuLabel;
                std::uint64_t user, nice, system, idle, iowait = 0, irq = 0, softirq = 0, steal = 0;

                if(!(file >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)){
                    outTotal = outBusy = 0;
                    return;
                }

                outTotal = user + nice + system + idle + iowait + irq + softirq + steal;
                outBusy = outTotal - (idle + iowait);
            }

            void read_system_memory_bytes(std::uint64_t& outUsed, std::uint64_t& outTotal){
                std::ifstream file("/proc/meminfo");
                std::string key, unit;
                std::uint64_t value = 0;
                std::uint64_t totalKb = 0, availableKb = 0;

                while(file >> key >> value >> unit){
                    if(key == "MemTotal:") totalKb = value;
                    else if(key == "MemAvailable:") availableKb = value;
                }

                outTotal = totalKb * 1024;
                outUsed = (totalKb - availableKb) * 1024;
            }
        #endif
    }

    ProfilerSystem::ProfilerSystem()
        : m_processCpuHistory(HISTORY_CAPACITY), m_systemCpuHistory(HISTORY_CAPACITY), m_processFrameTimeHistory(HISTORY_CAPACITY)
    {
    }

    void ProfilerSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        if(ImGui::IsKeyPressed(ImGuiKey_F3, false))
            m_visible = !m_visible;

        auto now = std::chrono::steady_clock::now();
        if(!m_hasPrevSample || now - m_lastSampleTime >= SAMPLE_INTERVAL)
            sample(now, dt);

        if(!m_visible)
            return;

        ImGui::SetNextWindowSize(ImVec2(320, 400), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Profiler", &m_visible)){
            ImGui::Text("Frame Time: %fs", m_processFrameTime);
            ImGui::Separator();
            ImGui::PlotLines("##ProcessFrameTimeHistory", get_history_value, &m_processFrameTimeHistory,
                (int)m_processCpuHistory.length(), 0, nullptr, 0.f, FLT_MAX, ImVec2(0, 48));
            ImGui::Text("Frames Per Second: %d", static_cast<int>(1.f / m_processFrameTime));
            ImGui::PlotLines("##ProcessFPSHistory", get_fps_history_value, &m_processFrameTimeHistory,
                (int)m_processCpuHistory.length(), 0, nullptr, 0.f, FLT_MAX, ImVec2(0, 48));
            ImGui::Spacing();

            ImGui::TextDisabled("Process");
            ImGui::Separator();
            ImGui::Text("CPU: %.1f%%", m_processCpuPercent);
            ImGui::PlotLines("##ProcessCpuHistory", get_history_value, &m_processCpuHistory,
                (int)m_processCpuHistory.length(), 0, nullptr, 0.f, 100.f, ImVec2(0, 48));
            ImGui::Text("Memory: %.1f MB", m_processMemoryBytes / (1024.0 * 1024.0));

            ImGui::Spacing();
            ImGui::TextDisabled("System");
            ImGui::Separator();
            ImGui::Text("CPU: %.1f%%", m_systemCpuPercent);
            ImGui::PlotLines("##SystemCpuHistory", get_history_value, &m_systemCpuHistory,
                (int)m_systemCpuHistory.length(), 0, nullptr, 0.f, 100.f, ImVec2(0, 48));
            ImGui::Text("Memory: %.1f / %.1f MB", m_systemUsedMemoryBytes / (1024.0 * 1024.0), m_systemTotalMemoryBytes / (1024.0 * 1024.0));
        }

        ImGui::End();
    }

    void ProfilerSystem::sample(std::chrono::steady_clock::time_point now, const Time& dt){
        std::uint64_t processCpuTicks = read_process_cpu_ticks();
        std::uint64_t systemTotalTicks, systemBusyTicks;
        read_system_cpu_ticks(systemTotalTicks, systemBusyTicks);

        if(m_hasPrevSample){
            double wallSeconds = std::chrono::duration<double>(now - m_lastSampleTime).count();
            if(wallSeconds > 0.0){
                double processSeconds = process_ticks_to_seconds(processCpuTicks - m_prevProcessCpuTicks);
                m_processCpuPercent = static_cast<float>(std::max(processSeconds / wallSeconds * 100.0, 0.0));
            }

            std::uint64_t totalDelta = systemTotalTicks - m_prevSystemTotalTicks;
            std::uint64_t busyDelta = systemBusyTicks - m_prevSystemBusyTicks;
            if(totalDelta > 0)
                m_systemCpuPercent = static_cast<float>(busyDelta) / static_cast<float>(totalDelta) * 100.f;
        }

        m_processMemoryBytes = read_process_memory_bytes();
        read_system_memory_bytes(m_systemUsedMemoryBytes, m_systemTotalMemoryBytes);

        m_processFrameTime = dt.as_seconds();

        m_processCpuHistory.push(m_processCpuPercent);
        m_systemCpuHistory.push(m_systemCpuPercent);
        m_processFrameTimeHistory.push(m_processFrameTime);

        m_prevProcessCpuTicks = processCpuTicks;
        m_prevSystemTotalTicks = systemTotalTicks;
        m_prevSystemBusyTicks = systemBusyTicks;
        m_lastSampleTime = now;
        m_hasPrevSample = true;
    }
}
