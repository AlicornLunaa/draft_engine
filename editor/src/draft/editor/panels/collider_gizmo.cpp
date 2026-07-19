#include "draft/editor/panels/collider_gizmo.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/physics/shapes/chain_shape.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/shapes/edge_shape.hpp"
#include "draft/physics/shapes/polygon_shape.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

#include <limits>

namespace Draft {
    namespace {
        constexpr float POINT_HANDLE_RADIUS = 6.f;
        constexpr float MIDPOINT_HANDLE_RADIUS = 5.f;
        constexpr float GHOST_HANDLE_RADIUS = 6.f;
        constexpr float GHOST_MAX_SCREEN_DISTANCE = 30.f;
        constexpr float POSITION_SNAP_STEP = 10.f;

        constexpr Vector4f POINT_COLOR{1.f, 0.784f, 0.f, 1.f};
        constexpr Vector4f POINT_HOVER_COLOR{1.f, 0.902f, 0.47f, 1.f};
        constexpr Vector4f MIDPOINT_COLOR{0.f, 0.784f, 1.f, 1.f};
        constexpr Vector4f MIDPOINT_HOVER_COLOR{0.588f, 0.902f, 1.f, 1.f};
        constexpr Vector4f GHOST_COLOR{1.f, 1.f, 1.f, 0.6f};

        float snap_to_step(float value, float step){
            return Math::round(value / step) * step;
        }

        Vector2f closest_point_on_segment(const Vector2f& point, const Vector2f& a, const Vector2f& b){
            Vector2f ab = b - a;
            float lengthSq = Math::dot(ab, ab);
            if(lengthSq < 1e-8f)
                return a;

            float t = Math::clamp(Math::dot(point - a, ab) / lengthSq, 0.f, 1.f);
            return a + ab * t;
        }

        EditablePointList make_polygon_point_list(PolygonShape* polygon){
            EditablePointList points;
            points.closed = true;
            points.count = [polygon]{ return polygon->get_vertex_count(); };
            points.get = [polygon](size_t i){ return polygon->get_vertex(i); };
            points.set = [polygon](size_t i, const Vector2f& v){ polygon->set_vertex(i, v); };

            // PolygonShape only has del_vertex(index) and an appending add_vertex() - no
            // insert-at-index, so inserting rebuilds the vertex list in the new order.
            points.insert = [polygon](size_t index, const Vector2f& v){
                std::vector<Vector2f> vertices = polygon->get_vertices();
                vertices.insert(vertices.begin() + static_cast<long>(index), v);

                while(polygon->get_vertex_count() > 0)
                    polygon->del_vertex(polygon->get_vertex_count() - 1);

                for(const Vector2f& vertex : vertices)
                    polygon->add_vertex(vertex);
            };

            points.remove = [polygon](size_t i){ polygon->del_vertex(i); };
            return points;
        }

        EditablePointList make_chain_point_list(ChainShape* chain){
            EditablePointList points;
            points.closed = (chain->get_chain_type() == ChainShape::LOOP);
            points.count = [chain]{ return chain->get_points().size(); };
            points.get = [chain](size_t i){ return chain->get_points()[i]; };

            // ChainShape has no in-place point setter/insert, only add()/remove()/clear(), so
            // both rebuild the whole list with the one change applied.
            points.set = [chain](size_t i, const Vector2f& v){
                std::vector<Vector2f> pts = chain->get_points();
                pts[i] = v;
                chain->clear();
                for(const Vector2f& p : pts) chain->add(p);
            };

            points.insert = [chain](size_t index, const Vector2f& v){
                std::vector<Vector2f> pts = chain->get_points();
                pts.insert(pts.begin() + static_cast<long>(index), v);
                chain->clear();
                for(const Vector2f& p : pts) chain->add(p);
            };

            points.remove = [chain](size_t i){ chain->remove(i); };
            return points;
        }

        EditablePointList make_edge_point_list(EdgeShape* edge){
            // A fixed pair of points, no insert/remove, since an edge can't grow past 2 points.
            EditablePointList points;
            points.closed = false;
            points.count = []{ return static_cast<size_t>(2); };
            points.get = [edge](size_t i){ return i == 0 ? edge->get_start() : edge->get_end(); };
            points.set = [edge](size_t i, const Vector2f& v){ if(i == 0) edge->set_start(v); else edge->set_end(v); };
            return points;
        }
    }

    ColliderGizmoSystem::ColliderGizmoSystem(EditorApplication& app) : m_app(app) {}

    RenderLayer ColliderGizmoSystem::get_render_layers() const { return RenderLayer::Interface; }

    void ColliderGizmoSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Interface)
            return;

        m_app.colliderGizmoActiveThisFrame = false;

        ColliderShapeSelection& selection = m_app.colliderShapeSelection;
        if(!selection.editing || selection.shapeIndex < 0)
            return;

        Entity entity = m_app.selection.get();
        if(!entity.is_valid() || !entity.has_component<ColliderComponent>() || !entity.has_component<TransformComponent>())
            return;

        Collider& collider = entity.get_component<ColliderComponent>().collider;
        if(selection.shapeIndex >= static_cast<int>(collider.get_shape_count())){
            selection.shapeIndex = -1;
            return;
        }

        Camera* camera = m_app.gameScene.get_active_camera();
        if(!camera)
            return;

        Shape* shape = collider.get_shapes()[selection.shapeIndex].get();
        const TransformComponent& transform = entity.get_component<TransformComponent>();
        GizmoViewport viewport{*camera, m_app.viewportScreenPosition, m_app.viewportSize};

        bool changed = false;

        if(ImGui::Begin("Viewport###Viewport")){
            switch(shape->type){
                case ShapeType::POLYGON: {
                    EditablePointList points = make_polygon_point_list(static_cast<PolygonShape*>(shape));
                    changed = draw_point_list_gizmo(points, transform, viewport);
                    break;
                }

                case ShapeType::CHAIN: {
                    EditablePointList points = make_chain_point_list(static_cast<ChainShape*>(shape));
                    changed = draw_point_list_gizmo(points, transform, viewport);
                    break;
                }

                case ShapeType::EDGE: {
                    EditablePointList points = make_edge_point_list(static_cast<EdgeShape*>(shape));
                    changed = draw_point_list_gizmo(points, transform, viewport);
                    break;
                }

                case ShapeType::CIRCLE:
                    changed = draw_circle_gizmo(*static_cast<CircleShape*>(shape), transform, viewport);
                    break;
            }
        }

        ImGui::End();

        if(changed && collider.is_attached())
            collider.update_collider();
    }

    bool ColliderGizmoSystem::draw_point_list_gizmo(EditablePointList& points, const TransformComponent& transform, const GizmoViewport& viewport){
        bool changed = false;
        size_t count = points.count();
        if(count == 0)
            return false;

        Matrix3 toWorld = transform.get_transform();
        Matrix3 toLocal = Math::inverse(toWorld);

        auto world_of = [&](size_t i){ return Vector2f(toWorld * Vector3f(points.get(i), 1.f)); };

        int removeIndex = -1;
        size_t minCount = points.closed ? 3 : 2;

        for(size_t i = 0; i < count; i++){
            Vector2f worldPos = world_of(i);
            Vector2f screenPos = viewport.world_to_screen(worldPos);

            std::string id = "Point" + std::to_string(i);
            HandleInteraction interaction = draw_circle_handle(id.c_str(), screenPos, POINT_HANDLE_RADIUS, POINT_COLOR, POINT_HOVER_COLOR);
            m_app.colliderGizmoActiveThisFrame |= interaction.active;

            if(interaction.rightClicked && points.remove && count > minCount)
                removeIndex = static_cast<int>(i);

            if(interaction.justActivated){
                m_dragPointIndex = static_cast<int>(i);
                m_dragPointWorldStart = worldPos;
                ImVec2 mousePos = ImGui::GetMousePos();
                m_dragMouseWorldStart = viewport.screen_to_world({mousePos.x, mousePos.y});
            }

            if(interaction.active && m_dragPointIndex == static_cast<int>(i) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
                ImVec2 mousePos = ImGui::GetMousePos();
                Vector2f mouseWorldNow = viewport.screen_to_world({mousePos.x, mousePos.y});
                Vector2f newWorld = m_dragPointWorldStart + (mouseWorldNow - m_dragMouseWorldStart);

                if(ImGui::GetIO().KeyShift)
                    newWorld = { snap_to_step(newWorld.x, POSITION_SNAP_STEP), snap_to_step(newWorld.y, POSITION_SNAP_STEP) };

                points.set(i, Vector2f(toLocal * Vector3f(newWorld, 1.f)));
                changed = true;
            }
        }

        if(removeIndex >= 0){
            points.remove(static_cast<size_t>(removeIndex));
            changed = true;
            count = points.count();
        }

        size_t edgeCount = points.closed ? count : (count > 0 ? count - 1 : 0);

        for(size_t i = 0; i < edgeCount; i++){
            size_t next = (i + 1) % count;
            Vector2f worldA = world_of(i);
            Vector2f worldB = world_of(next);
            Vector2f midWorld = (worldA + worldB) * 0.5f;
            Vector2f edgeDir = Math::normalize(worldB - worldA);
            Vector2f normal{edgeDir.y, -edgeDir.x};

            Vector2f midScreen = viewport.world_to_screen(midWorld);
            std::string id = "Midpoint" + std::to_string(i);
            HandleInteraction interaction = draw_circle_handle(id.c_str(), midScreen, MIDPOINT_HANDLE_RADIUS, MIDPOINT_COLOR, MIDPOINT_HOVER_COLOR);
            m_app.colliderGizmoActiveThisFrame |= interaction.active;

            if(interaction.justActivated){
                m_dragEdgeIndex = static_cast<int>(i);
                m_dragEdgeAWorldStart = worldA;
                m_dragEdgeBWorldStart = worldB;
                m_dragEdgeNormal = normal;
                ImVec2 mousePos = ImGui::GetMousePos();
                m_dragMouseWorldStart = viewport.screen_to_world({mousePos.x, mousePos.y});
            }

            if(interaction.active && m_dragEdgeIndex == static_cast<int>(i) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
                ImVec2 mousePos = ImGui::GetMousePos();
                Vector2f mouseWorldNow = viewport.screen_to_world({mousePos.x, mousePos.y});
                float distance = Math::dot(mouseWorldNow - m_dragMouseWorldStart, m_dragEdgeNormal);

                if(ImGui::GetIO().KeyShift)
                    distance = snap_to_step(distance, POSITION_SNAP_STEP);

                Vector2f newWorldA = m_dragEdgeAWorldStart + m_dragEdgeNormal * distance;
                Vector2f newWorldB = m_dragEdgeBWorldStart + m_dragEdgeNormal * distance;

                points.set(i, Vector2f(toLocal * Vector3f(newWorldA, 1.f)));
                points.set(next, Vector2f(toLocal * Vector3f(newWorldB, 1.f)));
                changed = true;
            }
        }

        // Ctrl+ghost insert with a preview point snapped to the closest edge, insert it there on
        // click.
        if(points.insert && ImGui::GetIO().KeyCtrl && !ImGui::IsAnyItemActive() && edgeCount > 0){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorld = viewport.screen_to_world({mousePos.x, mousePos.y});

            float bestScreenDist = std::numeric_limits<float>::max();
            size_t bestEdge = 0;
            Vector2f bestWorld{};

            for(size_t i = 0; i < edgeCount; i++){
                size_t next = (i + 1) % count;
                Vector2f closest = closest_point_on_segment(mouseWorld, world_of(i), world_of(next));
                Vector2f closestScreen = viewport.world_to_screen(closest);
                float screenDist = Math::length(closestScreen - Vector2f(mousePos.x, mousePos.y));

                if(screenDist < bestScreenDist){
                    bestScreenDist = screenDist;
                    bestEdge = i;
                    bestWorld = closest;
                }
            }

            if(bestScreenDist <= GHOST_MAX_SCREEN_DISTANCE){
                Vector2f ghostScreen = viewport.world_to_screen(bestWorld);
                ImU32 ghostColor = ImGui::ColorConvertFloat4ToU32(ImVec4(GHOST_COLOR.r, GHOST_COLOR.g, GHOST_COLOR.b, GHOST_COLOR.a));
                ImGui::GetWindowDrawList()->AddCircle(ImVec2(ghostScreen.x, ghostScreen.y), GHOST_HANDLE_RADIUS, ghostColor, 0, 2.f);

                m_app.colliderGizmoActiveThisFrame = true;

                if(m_app.viewportHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)){
                    points.insert(bestEdge + 1, Vector2f(toLocal * Vector3f(bestWorld, 1.f)));
                    changed = true;
                }
            }
        }

        return changed;
    }

    bool ColliderGizmoSystem::draw_circle_gizmo(CircleShape& circle, const TransformComponent& transform, const GizmoViewport& viewport){
        Matrix3 toWorld = transform.get_transform();
        Vector2f worldPos = Vector2f(toWorld * Vector3f(circle.get_position(), 1.f));
        Vector2f screenPos = viewport.world_to_screen(worldPos);

        HandleInteraction interaction = draw_circle_handle("ColliderCirclePosition", screenPos, POINT_HANDLE_RADIUS, POINT_COLOR, POINT_HOVER_COLOR);
        m_app.colliderGizmoActiveThisFrame |= interaction.active;

        if(interaction.justActivated){
            m_dragPointWorldStart = worldPos;
            ImVec2 mousePos = ImGui::GetMousePos();
            m_dragMouseWorldStart = viewport.screen_to_world({mousePos.x, mousePos.y});
        }

        if(interaction.active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 mousePos = ImGui::GetMousePos();
            Vector2f mouseWorldNow = viewport.screen_to_world({mousePos.x, mousePos.y});
            Vector2f newWorld = m_dragPointWorldStart + (mouseWorldNow - m_dragMouseWorldStart);

            if(ImGui::GetIO().KeyShift)
                newWorld = { snap_to_step(newWorld.x, POSITION_SNAP_STEP), snap_to_step(newWorld.y, POSITION_SNAP_STEP) };

            circle.set_position(Vector2f(Math::inverse(toWorld) * Vector3f(newWorld, 1.f)));
            return true;
        }

        return false;
    }
}
