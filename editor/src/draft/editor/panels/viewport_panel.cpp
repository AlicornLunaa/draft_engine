#include "draft/editor/panels/viewport_panel.hpp"
#include "draft/editor/asset_drag_drop.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/editor/prefab.hpp"
#include "draft/editor/project.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

#include <cstdint>
#include <string>

namespace Draft {
    ViewportPanelSystem::ViewportPanelSystem(EditorApplication& app) : m_app(app) {}

    void ViewportPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        if(!m_app.viewportPanelVisible){
            // Nothing draws the window this frame.
            m_app.viewportFocused = false;
            m_app.viewportHovered = false;
            return;
        }

        // Draws a viewport and lets the engine know if this special widget is currently focused
        // when it IS focused, all inputs should be forwarded to the game engine and skip the editor engine.
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        if(m_app.viewportHovered && m_app.viewportFocused){
            flags |= ImGuiWindowFlags_NoResize;
            flags |= ImGuiWindowFlags_NoMove;
        }

        ImGui::SetNextWindowSize({640, 480}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Viewport###Viewport", &m_app.viewportPanelVisible, flags)){
            ImVec2 regionAvailable = ImGui::GetContentRegionAvail();
            ImVec2 cursorPosition = ImGui::GetCursorScreenPos();
            ImVec2 mousePosition = ImGui::GetMousePos();

            // Display gameApp's current output texture as-is.
            auto textureId = (ImTextureID)(intptr_t)m_app.gameApp.get_output().get_texture_handle();
            ImGui::Image(textureId, regionAvailable, ImVec2(0, 1), ImVec2(1, 0));

            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(asset_drag_payload_type(AssetKind::Prefab))){
                    std::string key(static_cast<const char*>(payload->Data), static_cast<size_t>(payload->DataSize));

                    if(Camera* camera = m_app.gameScene.get_active_camera(); camera && m_app.has_project()){
                        GizmoViewport viewport{*camera, {cursorPosition.x, cursorPosition.y}, {regionAvailable.x, regionAvailable.y}};
                        ImVec2 dropScreenPos = ImGui::GetMousePos();
                        Vector2f worldPos = viewport.screen_to_world({dropScreenPos.x, dropScreenPos.y});

                        Entity instance = PrefabManager(m_app).instantiate_prefab(m_app.project()->root() / key, worldPos);
                        if(instance.is_valid())
                            m_app.selection.set(instance);
                    }
                }

                ImGui::EndDragDropTarget();
            }

            if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
                ImGui::SetWindowFocus();

            m_app.viewportFocused = ImGui::IsWindowFocused();
            m_app.viewportHovered = ImGui::IsItemHovered();
            m_regionLocalCursorPosition = {mousePosition.x - cursorPosition.x, mousePosition.y - cursorPosition.y};
            m_regionScreenPosition = {cursorPosition.x, cursorPosition.y};
            m_regionAvailable = Math::max({regionAvailable.x, regionAvailable.y}, Vector2u(1, 1));

            m_app.viewportScreenPosition = m_regionScreenPosition;
            m_app.viewportSize = m_regionAvailable;

            // Dispatch event for mouse hover/focus events
            if(m_regionAvailable.x != m_regionAvailableLast.x || m_regionAvailable.y != m_regionAvailableLast.y){
                Event event;
                event.type = Event::Resized;
                event.size.width = m_regionAvailable.x;
                event.size.height = m_regionAvailable.y;
                m_app.pendingViewportEvents.push(event);
            }

            if(m_app.viewportHovered && !m_regionHoveredLast){
                // Region has just been hovered
                m_app.pendingViewportEvents.push({.type = Event::MouseEntered});
            } else if(!m_app.viewportHovered && m_regionHoveredLast){
                // Region just left hover
                m_app.pendingViewportEvents.push({.type = Event::MouseLeft});
            }

            if(m_app.viewportFocused && !m_regionFocusedLast){
                // Region has just been focused
                m_app.pendingViewportEvents.push({.type = Event::GainedFocus});
            } else if(!m_app.viewportFocused && m_regionFocusedLast){
                // Region just left focused
                m_app.pendingViewportEvents.push({.type = Event::LostFocus});
            }

            m_regionAvailableLast = m_regionAvailable;
            m_regionHoveredLast = m_app.viewportHovered;
            m_regionFocusedLast = m_app.viewportFocused;
        }

        ImGui::End();
    }

    bool ViewportPanelSystem::on_event(const Event& event){
        if(event.type == Event::Resized){
            // The subapp resize is handled in the editor application
            return false; // Never consume a resize, everything should know about it
        }

        // Other events should only be shuttled through if this item is hovered
        if(m_app.viewportFocused){
            // Translate screen-space coordinates to be relative to the viewport image's origin
            Event localEvent = event;

            switch(event.type){
                case Event::MouseMoved:
                    if(!m_app.viewportHovered) return false; // Bail out even when focused
                    localEvent.mouseMove.x = (int)(event.mouseMove.x - m_regionScreenPosition.x);
                    localEvent.mouseMove.y = (int)(event.mouseMove.y - m_regionScreenPosition.y);
                    break;

                case Event::MouseButtonPressed:
                case Event::MouseButtonReleased:
                    if(!m_app.viewportHovered) return false; // Bail out even when focused
                    localEvent.mouseButton.x = (int)(event.mouseButton.x - m_regionScreenPosition.x);
                    localEvent.mouseButton.y = (int)(event.mouseButton.y - m_regionScreenPosition.y);
                    break;

                case Event::MouseWheelScrolled:
                    if(!m_app.viewportHovered) return false; // Bail out even when focused
                    break;

                default:
                    break;
            }

            m_app.pendingViewportEvents.push(localEvent);
            return true; // Any other event that doesn't bail out should be consumed
        }

        return false;
    }
}
