#include "draft/core/application.hpp"
#include "draft/widgets/stats.hpp"
#include "draft/input/keyboard.hpp"
#include "imgui.h"

#include <algorithm>
#include <fstream>

using namespace std;

namespace Draft {
    void process_mem_usage(float& vmUsage, float& residentSet){
        vmUsage     = 0.0;
        residentSet = 0.0;

        ifstream statStream("/proc/self/stat", ios_base::in);

        // dummy vars for leading entries in stat that we don't care about
        string pid, comm, state, ppid, pgrp, session, tty_nr;
        string tpgid, flags, minflt, cminflt, majflt, cmajflt;
        string utime, stime, cutime, cstime, priority, nice;
        string O, itrealvalue, starttime;

        // the two fields we want
        unsigned long vsize;
        long rss;

        statStream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                    >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                    >> utime >> stime >> cutime >> cstime >> priority >> nice
                    >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

        statStream.close();

        long pageSizeKb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
        vmUsage     = vsize / 1024.0;
        residentSet = rss * pageSizeKb / 1024.f;
    }

    void Stats::draw(Application& app){
        if(app.debug){
            // Get stuff
            auto frameTime = app.deltaTime.as_seconds();
            auto fps = 1.f / frameTime;

            float vmUsage, usage;
            process_mem_usage(vmUsage, usage);

            updateTimer += app.deltaTime.as_seconds();
            if(updateTimer > 0.5f){
                maxUsage = 0.f;
                maxFps = 0.f;

                for(int i = 0; i < samples - 1; i++){;
                    memoryUsage[i] = memoryUsage[i + 1];
                    fpsOverTime[i] = fpsOverTime[i + 1];
                    maxUsage = std::max(memoryUsage[i], maxUsage);
                    maxFps = std::max(fpsOverTime[i], maxFps);
                }

                memoryUsage[samples - 1] = usage;
                fpsOverTime[samples - 1] = fps;
                updateTimer = 0.f;
            }

            // Draw frame
            ImGui::Begin("Statistics");
            ImGui::Text("FPS: %f", fps);
            ImGui::Text("Frame Time: %f", frameTime);
            ImGui::Text("Time Step: %f", app.timeStep.as_seconds());
            ImGui::PlotLines("FPS", &fpsOverTime[0], samples, 0, nullptr, 0.f, maxFps, {0, 80});
            ImGui::Text("Memory (mb): %f", usage);
            ImGui::PlotLines("(mb)", &memoryUsage[0], samples, 0, nullptr, 0.f, maxUsage, {0, 80});
            ImGui::Checkbox("Debug", &app.debug);

            if(ImGui::Button("Reload Assets")){
                app.console.run("reload_assets");
            }

            ImGui::End();
        }

        // Handle pressing keys
        if(app.keyboard.is_just_pressed(Keyboard::F3)){
            app.debug = !app.debug;
        }
    }
}