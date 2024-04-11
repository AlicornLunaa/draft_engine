#include "draft/core/application.hpp"
#include "draft/widgets/stats.hpp"
#include "imgui.h"

using namespace std;

namespace Draft {
    void Stats::draw(Application& app){
        // Get stuff
        auto frameTime = app.deltaTime.as_seconds();
        auto fps = 1.f / frameTime;

        // Draw frame
        ImGui::Begin("Statistics");
        ImGui::Text("FPS: %f", fps);
        ImGui::Text("Frame Time: %f", frameTime);

        if(ImGui::Button("Reload Assets")){
            app.console.run("reload_assets");
        }

        ImGui::End();
    }
}