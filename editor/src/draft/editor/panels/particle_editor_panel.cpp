#include "draft/editor/panels/particle_editor_panel.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/resource_serializer.hpp" // IWYU pragma: keep

#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>

namespace Draft {
    namespace {
        constexpr float PARTICLE_EMIT_INTERVAL = 0.05f;
    }

    ParticleEditorPanelSystem::ParticleEditorPanelSystem(EditorApplication& app)
        : m_app(app), m_previewTarget(FramebufferProperties{.size = {256, 256}})
    {
    }

    void ParticleEditorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.particleEditorPanelVisible)
            return;

        std::string title = "Particle Editor";
        if(m_app.particleEditorAssetKey)
            title += " - " + std::filesystem::path(*m_app.particleEditorAssetKey).filename().string();
        title += "###ParticleEditor";

        ImGui::SetNextWindowSize({420, 560}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin(title.c_str(), &m_app.particleEditorPanelVisible)){
            if(!m_app.particleEditorAssetKey){
                ImGui::TextDisabled("Double click a .particle asset in the Asset Browser to edit it.");
            } else {
                if(m_loadedKey != *m_app.particleEditorAssetKey)
                    load_asset(*m_app.particleEditorAssetKey);

                if(!m_resource.is_valid()){
                    ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Failed to load %s", m_loadedKey.c_str());
                } else {
                    ParticleProps& props = *m_resource.get();

                    constexpr float propertiesHeight = 320.f;
                    float previewHeight = std::max(ImGui::GetContentRegionAvail().y - propertiesHeight - ImGui::GetStyle().ItemSpacing.y, 100.f);

                    draw_preview(dt, props, previewHeight);
                    ImGui::Separator();

                    ImGui::BeginChild("ParticleEditorProperties");
                    draw_properties(props);
                    ImGui::Separator();
                    if(ImGui::Button("Save"))
                        save_asset();
                    ImGui::EndChild();
                }
            }
        }

        ImGui::End();
    }

    void ParticleEditorPanelSystem::load_asset(const std::string& key){
        m_loadedKey = key;
        m_particleSystem = ParticleSystem();
        m_emitAccumulator = 0.f;

        try {
            m_resource = m_app.assets.get<ParticleProps>(key);
        } catch(const std::exception& e){
            m_resource = Resource<ParticleProps>();
            Logger::println(LogLevel::Severe, "ParticleEditor", "Failed to load " + key + ": " + e.what());
        }
    }

    void ParticleEditorPanelSystem::save_asset(){
        if(!m_resource.is_valid() || !m_app.has_project())
            return;

        SceneSerializationContext ctx;
        ctx.assets = &m_app.assets;
        Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

        JSON json = JSON::object();
        Serializer::serialize(*m_resource.get(), json);

        HostFileSystem().open(m_app.project()->root() / m_loadedKey).write_string(json.dump(4));
    }

    void ParticleEditorPanelSystem::draw_preview(Time dt, ParticleProps& props, float availableHeight){
        ImGui::SetNextItemWidth(120.f);
        ImGui::DragFloat("Preview Zoom", &m_previewExtent, 0.05f, 0.1f, 20.f);

        float rowWidth = ImGui::GetContentRegionAvail().x;
        float imageHeight = availableHeight - ImGui::GetFrameHeightWithSpacing();
        float squareSize = std::clamp(std::min(rowWidth, imageHeight), 64.f, 1024.f);

        Vector2u desiredSize{(unsigned int)squareSize, (unsigned int)squareSize};
        if(m_previewTarget.get_size() != desiredSize)
            m_previewTarget.set_size(desiredSize);

        m_emitAccumulator += dt.as_seconds();
        while(m_emitAccumulator >= PARTICLE_EMIT_INTERVAL){
            m_particleSystem.emit(props);
            m_emitAccumulator -= PARTICLE_EMIT_INTERVAL;
        }
        m_particleSystem.update(dt);

        Camera camera = Camera::make_orthographic(
            {0, 0, 10}, {0, 0, -1},
            -m_previewExtent, m_previewExtent, -m_previewExtent, m_previewExtent
        );

        m_previewTarget.begin();
        m_previewBatch.set_trans_matrix(camera.get_view());
        m_previewBatch.set_proj_matrix(camera.get_projection());
        m_particleSystem.render(m_previewBatch);
        m_previewBatch.flush();
        m_previewTarget.end();

        float centerOffset = (rowWidth - squareSize) * 0.5f;
        if(centerOffset > 0.f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);

        ImTextureID textureId = static_cast<ImTextureID>(static_cast<intptr_t>(m_previewTarget.get_texture().get_texture_handle()));
        ImGui::Image(textureId, ImVec2(squareSize, squareSize), ImVec2(0, 1), ImVec2(1, 0));
    }

    void ParticleEditorPanelSystem::draw_properties(ParticleProps& props){
        FieldContext ctx{m_app.gameScene, m_app.assets, m_app.selection, m_app};

        for_each_field(props, [&](std::string_view name, auto& field){
            draw_field(ctx, name, field);
        });
    }
}
