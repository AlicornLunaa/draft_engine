#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/circular_buffer.hpp"
#include "draft/util/reflectable.hpp"

#include <chrono>
#include <cstdint>

namespace Draft {
    /**
     * @brief Process/system CPU and RAM usage overlay, hidden by default and toggled with F3
     * mirrors ConsoleSystem's self-contained visibility. Samples the OS roughly twice a second
     * rather than every frame, since the underlying counters don't change meaningfully faster
     * than that and re-reading them (especially /proc on Linux) isn't free.
     */
    class ProfilerSystem : public AbstractSystem {
    public:
        ProfilerSystem();

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(ProfilerSystem)

    private:
        void sample(std::chrono::steady_clock::time_point now, const Time& dt);

        bool m_visible = false;
        bool m_hasPrevSample = false;
        std::chrono::steady_clock::time_point m_lastSampleTime;

        // Raw OS counters from the previous sample, used to turn a cumulative counter into a
        // delta-over-time percentage on the next one.
        std::uint64_t m_prevProcessCpuTicks = 0;
        std::uint64_t m_prevSystemTotalTicks = 0;
        std::uint64_t m_prevSystemBusyTicks = 0;

        float m_processFrameTime = 0.f;
        float m_processCpuPercent = 0.f;
        float m_systemCpuPercent = 0.f;
        std::uint64_t m_processMemoryBytes = 0;
        std::uint64_t m_systemUsedMemoryBytes = 0;
        std::uint64_t m_systemTotalMemoryBytes = 0;

        CircularBuffer<float> m_processFrameTimeHistory;
        CircularBuffer<float> m_processCpuHistory;
        CircularBuffer<float> m_systemCpuHistory;
    };
}
