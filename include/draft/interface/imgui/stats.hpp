#pragma once

namespace Draft {
    class Application;

    struct Stats {
    private:
        // Variables
        constexpr static int samples = 100;
        constexpr static int fpsSamples = 20;

        float updateTimer = 0.f;
        float memoryUsage[samples] = { 0 };
        float fpsOverTime[samples] = { 0 };
        float avgFpsSamples[fpsSamples] = { 0 };
        float maxUsage = 0.f;
        float maxFps = 0.f;
        int sampleTimer = 0;

    public:
        // Functions
        void draw(Application& app);
    };
}