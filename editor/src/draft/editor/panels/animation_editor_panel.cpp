#include "draft/editor/panels/animation_editor_panel.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/material.hpp"
#include "draft/util/logger.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <vector>

namespace Draft {
    AnimationEditorPanelSystem::AnimationEditorPanelSystem(EditorApplication& app)
        : m_app(app), m_previewTarget(FramebufferProperties{.size = {256, 256}})
    {
    }

    void AnimationEditorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.animationEditorPanelVisible)
            return;

        std::string title = "Animation Editor";
        if(m_app.animationEditorAssetKey)
            title += " - " + std::filesystem::path(*m_app.animationEditorAssetKey).filename().string();
        title += "###AnimationEditor";

        ImGui::SetNextWindowSize({480, 420}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin(title.c_str(), &m_app.animationEditorPanelVisible)){
            if(!m_app.animationEditorAssetKey){
                ImGui::TextDisabled("Double click an animation asset in the Asset Browser to preview it.");
            } else {
                if(m_loadedKey != *m_app.animationEditorAssetKey)
                    load_asset(*m_app.animationEditorAssetKey);

                if(!m_resource.is_valid()){
                    ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Failed to load %s", m_loadedKey.c_str());
                } else {
                    Animation& animation = *m_resource.get();

                    if(!animation.get_tags().empty()){
                        draw_tag_list(animation);
                        ImGui::SameLine();
                    }

                    ImGui::BeginGroup();
                    draw_preview(dt, animation, ImGui::GetContentRegionAvail().y);
                    ImGui::EndGroup();
                }
            }
        }

        ImGui::End();
    }

    void AnimationEditorPanelSystem::load_asset(const std::string& key){
        m_loadedKey = key;
        m_selectedTag.clear();
        m_frameTime = 0.f;

        try {
            m_resource = m_app.assets.get<Animation>(key);
        } catch(const std::exception& e){
            m_resource = Resource<Animation>();
            Logger::println(LogLevel::Severe, "AnimationEditor", "Failed to load " + key + ": " + e.what());
        }
    }

    void AnimationEditorPanelSystem::draw_tag_list(Animation& animation){
        std::vector<std::string> tagNames;
        for(const auto& [name, tag] : animation.get_tags())
            tagNames.push_back(name);
        std::sort(tagNames.begin(), tagNames.end());

        ImGui::BeginGroup();
        ImGui::TextDisabled("Tags");
        ImGui::Separator();

        ImGui::BeginChild("##AnimationTagList", ImVec2(140.f, 0.f), ImGuiChildFlags_Borders);

        if(ImGui::Selectable("(All Frames)", m_selectedTag.empty())){
            m_selectedTag.clear();
            m_frameTime = 0.f;
        }

        for(const std::string& name : tagNames){
            if(ImGui::Selectable(name.c_str(), m_selectedTag == name)){
                m_selectedTag = name;
                m_frameTime = 0.f;
            }
        }

        ImGui::EndChild();
        ImGui::EndGroup();
    }

    void AnimationEditorPanelSystem::draw_preview(Time dt, Animation& animation, float availableHeight){
        std::string label = m_selectedTag.empty() ? "All Frames" : m_selectedTag;
        ImGui::Text("Showing: %s", label.c_str());

        ImGui::SetNextItemWidth(120.f);
        ImGui::DragFloat("Preview Zoom", &m_previewExtent, 0.05f, 0.1f, 20.f);

        float rowWidth = ImGui::GetContentRegionAvail().x;
        float imageHeight = availableHeight - ImGui::GetFrameHeightWithSpacing() * 2.f;
        float squareSize = std::clamp(std::min(rowWidth, imageHeight), 64.f, 1024.f);

        Vector2u desiredSize{(unsigned int)squareSize, (unsigned int)squareSize};
        if(m_previewTarget.get_size() != desiredSize)
            m_previewTarget.set_size(desiredSize);

        m_frameTime += dt.as_milliseconds();

        TextureRegion region = m_selectedTag.empty()
            ? animation.get_frame(m_frameTime)
            : animation.get_frame(m_selectedTag, m_frameTime);

        Camera camera = Camera::make_orthographic(
            {0, 0, 10}, {0, 0, -1},
            -m_previewExtent, m_previewExtent, m_previewExtent, -m_previewExtent
        );

        m_previewTarget.begin();
        m_previewBatch.set_trans_matrix(camera.get_view());
        m_previewBatch.set_proj_matrix(camera.get_projection());

        // Preserve the frame's own aspect ratio instead of stretching it to fill the square
        // preview, letterboxing whichever axis is shorter.
        float aspect = region.bounds.height > 0.f ? region.bounds.width / region.bounds.height : 1.f;
        Vector2f size = aspect >= 1.f
            ? Vector2f(m_previewExtent * 2.f, m_previewExtent * 2.f / aspect)
            : Vector2f(m_previewExtent * 2.f * aspect, m_previewExtent * 2.f);

        Material2D material;
        material.baseTexture = region.texture.get();

        m_previewBatch.draw(SpriteProps{
            {0, 0}, 0.f, size, size * 0.5f, 0.f, region.bounds, material
        });
        m_previewBatch.flush();
        m_previewTarget.end();

        float centerOffset = (rowWidth - squareSize) * 0.5f;
        if(centerOffset > 0.f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);

        ImTextureID textureId = static_cast<ImTextureID>(static_cast<intptr_t>(m_previewTarget.get_texture().get_texture_handle()));
        ImGui::Image(textureId, ImVec2(squareSize, squareSize), ImVec2(0, 1), ImVec2(1, 0));
    }
}
