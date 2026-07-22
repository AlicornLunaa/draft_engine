#include "draft/editor/panels/joint_gizmo.hpp"
#include "draft/components/joint_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/panels/gizmo_primitives.hpp"
#include "draft/rendering/camera.hpp"

#include "imgui.h"

namespace Draft {
    namespace {
        constexpr float ANCHOR_RADIUS = 5.f;
        constexpr float MAIN_ANCHOR_RADIUS = 6.f;
        constexpr float WORLD_ARC_RADIUS = 0.6f;
        constexpr float AXIS_PREVIEW_LENGTH = 1.5f;
        constexpr float UNLIMITED_THRESHOLD = 100000.f;
        constexpr int ARC_SEGMENTS = 24;

        constexpr Vector4f ANCHOR_A_COLOR{0.98f, 0.55f, 0.15f, 1.f};
        constexpr Vector4f ANCHOR_B_COLOR{0.25f, 0.75f, 0.95f, 1.f};
        constexpr Vector4f LINK_COLOR{0.9f, 0.9f, 0.9f, 0.55f};
        constexpr Vector4f LIMIT_COLOR{1.f, 0.85f, 0.2f, 0.85f};
        constexpr Vector4f RANGE_COLOR{1.f, 0.85f, 0.2f, 0.35f};
        constexpr Vector4f AXIS_COLOR{0.6f, 0.45f, 1.f, 0.9f};
        constexpr Vector4f GROUND_COLOR{0.5f, 0.85f, 0.3f, 1.f};
        constexpr Vector4f TARGET_COLOR{1.f, 0.3f, 0.3f, 1.f};
        constexpr Vector4f MOTOR_COLOR{1.f, 0.55f, 0.85f, 1.f};

        ImU32 pack(const Vector4f& c){ return ImGui::ColorConvertFloat4ToU32(ImVec4(c.r, c.g, c.b, c.a)); }

        Vector2f local_to_world(const TransformComponent& transform, const Vector2f& local){
            return Vector2f(transform.get_transform() * Vector3f(local, 1.f));
        }

        // Rotates a direction (not a point) by transform's rotation only, ignoring translation.
        Vector2f local_dir_to_world(const TransformComponent& transform, const Vector2f& dir){
            return Vector2f(transform.get_transform() * Vector3f(dir, 0.f));
        }

        void draw_anchor(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& worldPos, const Vector4f& color, float radius = ANCHOR_RADIUS){
            Vector2f s = viewport.world_to_screen(worldPos);
            dl->AddCircleFilled(ImVec2(s.x, s.y), radius, pack(color));
            dl->AddCircle(ImVec2(s.x, s.y), radius, IM_COL32(0, 0, 0, 255), 0, 1.5f);
        }

        void draw_link(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& worldA, const Vector2f& worldB, const Vector4f& color, float thickness = 2.f){
            Vector2f sa = viewport.world_to_screen(worldA);
            Vector2f sb = viewport.world_to_screen(worldB);
            dl->AddLine(ImVec2(sa.x, sa.y), ImVec2(sb.x, sb.y), pack(color), thickness);
        }

        void draw_cross(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& worldPos, const Vector4f& color, float size = 7.f){
            Vector2f s = viewport.world_to_screen(worldPos);
            ImU32 c = pack(color);
            dl->AddLine(ImVec2(s.x - size, s.y - size), ImVec2(s.x + size, s.y + size), c, 2.f);
            dl->AddLine(ImVec2(s.x - size, s.y + size), ImVec2(s.x + size, s.y - size), c, 2.f);
        }

        void draw_square(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& worldPos, const Vector4f& color, float size = 5.f){
            Vector2f s = viewport.world_to_screen(worldPos);
            dl->AddRectFilled(ImVec2(s.x - size, s.y - size), ImVec2(s.x + size, s.y + size), pack(color));
            dl->AddRect(ImVec2(s.x - size, s.y - size), ImVec2(s.x + size, s.y + size), IM_COL32(0, 0, 0, 255));
        }

        // Samples world-space points so the arc/circle stays correct under any camera zoom/rotation,
        // rather than approximating an ellipse in screen space.
        void draw_world_arc(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& center, float radius, float startAngle, float endAngle, const Vector4f& color, float thickness = 2.f){
            if(endAngle < startAngle){
                float tmp = startAngle;
                startAngle = endAngle;
                endAngle = tmp;
            }

            ImU32 c = pack(color);
            Vector2f prev = viewport.world_to_screen(center + Vector2f{Math::cos(startAngle), Math::sin(startAngle)} * radius);

            for(int i = 1; i <= ARC_SEGMENTS; i++){
                float t = startAngle + (endAngle - startAngle) * (static_cast<float>(i) / ARC_SEGMENTS);
                Vector2f next = viewport.world_to_screen(center + Vector2f{Math::cos(t), Math::sin(t)} * radius);
                dl->AddLine(ImVec2(prev.x, prev.y), ImVec2(next.x, next.y), c, thickness);
                prev = next;
            }
        }

        void draw_radial_tick(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& center, float radius, float angle, const Vector4f& color){
            Vector2f tip = center + Vector2f{Math::cos(angle), Math::sin(angle)} * radius;
            draw_link(dl, viewport, center, tip, color, 1.5f);
        }

        void draw_angle_limit_gizmo(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& center, float baseAngle, float lowerAngle, float upperAngle){
            float lower = baseAngle + lowerAngle;
            float upper = baseAngle + upperAngle;
            draw_world_arc(dl, viewport, center, WORLD_ARC_RADIUS, lower, upper, LIMIT_COLOR);
            draw_radial_tick(dl, viewport, center, WORLD_ARC_RADIUS, lower, LIMIT_COLOR);
            draw_radial_tick(dl, viewport, center, WORLD_ARC_RADIUS, upper, LIMIT_COLOR);
        }

        void draw_translation_limit_gizmo(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& center, const Vector2f& axisDir, float lower, float upper, bool limited){
            float previewLo = limited ? lower : -AXIS_PREVIEW_LENGTH;
            float previewHi = limited ? upper : AXIS_PREVIEW_LENGTH;

            Vector2f from = center + axisDir * previewLo;
            Vector2f to = center + axisDir * previewHi;
            draw_link(dl, viewport, from, to, AXIS_COLOR, limited ? 2.5f : 1.5f);

            if(limited){
                Vector2f normal{-axisDir.y, axisDir.x};
                float tickSize = WORLD_ARC_RADIUS * 0.35f;
                draw_link(dl, viewport, from - normal * tickSize, from + normal * tickSize, LIMIT_COLOR, 2.f);
                draw_link(dl, viewport, to - normal * tickSize, to + normal * tickSize, LIMIT_COLOR, 2.f);
            }
        }

        void draw_motor_arc_arrow(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& center, float motorSpeed){
            float dir = motorSpeed >= 0.f ? 1.f : -1.f;
            float start = 0.f;
            float end = dir * Math::radians(50.f);
            float radius = WORLD_ARC_RADIUS * 1.4f;

            draw_world_arc(dl, viewport, center, radius, Math::min(start, end), Math::max(start, end), MOTOR_COLOR, 2.5f);

            Vector2f tip = center + Vector2f{Math::cos(end), Math::sin(end)} * radius;
            Vector2f tangent{-Math::sin(end) * dir, Math::cos(end) * dir};
            Vector2f screenTip = viewport.world_to_screen(tip);
            Vector2f screenBack = viewport.world_to_screen(tip - tangent * (WORLD_ARC_RADIUS * 0.35f));
            Vector2f perp = Math::normalize(screenTip - screenBack);
            Vector2f side{-perp.y, perp.x};
            ImU32 c = pack(MOTOR_COLOR);
            dl->AddTriangleFilled(ImVec2(screenTip.x, screenTip.y),
                ImVec2(screenBack.x + side.x * 5.f, screenBack.y + side.y * 5.f),
                ImVec2(screenBack.x - side.x * 5.f, screenBack.y - side.y * 5.f), c);
        }

        void draw_motor_linear_arrow(ImDrawList* dl, const GizmoViewport& viewport, const Vector2f& worldPos, const Vector2f& axisDir, float motorSpeed){
            Vector2f dir = axisDir * (motorSpeed >= 0.f ? 1.f : -1.f);
            Vector2f tip = worldPos + dir * (WORLD_ARC_RADIUS * 0.9f);
            Vector2f screenTip = viewport.world_to_screen(tip);
            Vector2f screenBase = viewport.world_to_screen(worldPos + dir * (WORLD_ARC_RADIUS * 0.4f));
            Vector2f perp = Math::normalize(screenTip - screenBase);
            Vector2f side{-perp.y, perp.x};
            ImU32 c = pack(MOTOR_COLOR);
            dl->AddLine(ImVec2(screenBase.x, screenBase.y), ImVec2(screenTip.x, screenTip.y), c, 2.5f);
            dl->AddTriangleFilled(ImVec2(screenTip.x, screenTip.y),
                ImVec2(screenTip.x - perp.x * 8.f + side.x * 5.f, screenTip.y - perp.y * 8.f + side.y * 5.f),
                ImVec2(screenTip.x - perp.x * 8.f - side.x * 5.f, screenTip.y - perp.y * 8.f - side.y * 5.f), c);
        }

        const TransformComponent* body_transform(Entity entity){
            if(!entity.is_valid())
                return nullptr;

            return entity.try_get_component<TransformComponent>();
        }

        void draw_two_body_joint(ImDrawList* dl, const GizmoViewport& viewport, const TransformComponent* transformA, const TransformComponent* transformB, const Vector2f& localAnchorA, const Vector2f& localAnchorB){
            if(transformA)
                draw_anchor(dl, viewport, local_to_world(*transformA, localAnchorA), ANCHOR_A_COLOR, MAIN_ANCHOR_RADIUS);

            if(transformB)
                draw_anchor(dl, viewport, local_to_world(*transformB, localAnchorB), ANCHOR_B_COLOR);

            if(transformA && transformB)
                draw_link(dl, viewport, local_to_world(*transformA, localAnchorA), local_to_world(*transformB, localAnchorB), LINK_COLOR);
        }
    }

    JointGizmoSystem::JointGizmoSystem(EditorApplication& app) : m_app(app) {}

    RenderLayer JointGizmoSystem::get_render_layers() const { return RenderLayer::Interface; }

    void JointGizmoSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Interface || !m_app.viewportPanelVisible)
            return;

        Entity selected = m_app.selection.get();
        if(!selected.is_valid())
            return;

        Camera* camera = m_app.gameScene.get_active_camera();
        if(!camera)
            return;

        GizmoViewport viewport{*camera, m_app.viewportScreenPosition, m_app.viewportSize};

        if(!ImGui::Begin("Viewport###Viewport")){
            ImGui::End();
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();

        if(auto* joint = selected.try_get_component<RevoluteJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);
            draw_two_body_joint(dl, viewport, transformA, transformB, joint->localAnchorA, joint->localAnchorB);

            if(transformA){
                Vector2f center = local_to_world(*transformA, joint->localAnchorA);
                float baseAngle = transformA->rotation + joint->referenceAngle;

                if(joint->enableLimit)
                    draw_angle_limit_gizmo(dl, viewport, center, baseAngle, joint->lowerAngle, joint->upperAngle);

                if(joint->enableMotor && joint->motorSpeed != 0.f)
                    draw_motor_arc_arrow(dl, viewport, center, joint->motorSpeed);
            }
        } else if(auto* joint = selected.try_get_component<DistanceJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);
            draw_two_body_joint(dl, viewport, transformA, transformB, joint->anchorA, joint->anchorB);

            if(transformA){
                Vector2f center = local_to_world(*transformA, joint->anchorA);
                if(joint->minLength > 0.f)
                    draw_world_arc(dl, viewport, center, joint->minLength, 0.f, Math::radians(360.f), RANGE_COLOR, 1.5f);
                if(joint->maxLength < UNLIMITED_THRESHOLD)
                    draw_world_arc(dl, viewport, center, joint->maxLength, 0.f, Math::radians(360.f), RANGE_COLOR, 1.5f);
            }
        } else if(auto* joint = selected.try_get_component<FrictionJointComponent>()){
            draw_two_body_joint(dl, viewport, body_transform(joint->entityA), body_transform(joint->entityB), joint->anchorA, joint->anchorB);
        } else if(auto* joint = selected.try_get_component<WeldJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);
            draw_two_body_joint(dl, viewport, transformA, transformB, joint->anchorA, joint->anchorB);

            if(transformA){
                Vector2f center = local_to_world(*transformA, joint->anchorA);
                draw_radial_tick(dl, viewport, center, WORLD_ARC_RADIUS * 0.7f, transformA->rotation + joint->referenceAngle, LINK_COLOR);
            }
        } else if(auto* joint = selected.try_get_component<PrismaticJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);
            draw_two_body_joint(dl, viewport, transformA, transformB, joint->anchorA, joint->anchorB);

            if(transformA){
                Vector2f center = local_to_world(*transformA, joint->anchorA);
                Vector2f axisDir = Math::normalize(local_dir_to_world(*transformA, joint->localAxisA));
                draw_translation_limit_gizmo(dl, viewport, center, axisDir, joint->lowerTranslation, joint->upperTranslation, joint->enableLimit);

                if(joint->enableMotor && joint->motorSpeed != 0.f)
                    draw_motor_linear_arrow(dl, viewport, center, axisDir, joint->motorSpeed);
            }
        } else if(auto* joint = selected.try_get_component<WheelJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);
            draw_two_body_joint(dl, viewport, transformA, transformB, joint->anchorA, joint->anchorB);

            if(transformA){
                Vector2f center = local_to_world(*transformA, joint->anchorA);
                Vector2f axisDir = Math::normalize(local_dir_to_world(*transformA, joint->localAxis));
                draw_translation_limit_gizmo(dl, viewport, center, axisDir, joint->lowerTranslation, joint->upperTranslation, joint->enableLimit);

                if(joint->enableMotor && joint->motorSpeed != 0.f)
                    draw_motor_linear_arrow(dl, viewport, center, axisDir, joint->motorSpeed);
            }
        } else if(auto* joint = selected.try_get_component<PulleyJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);

            draw_square(dl, viewport, joint->groundAnchorA, GROUND_COLOR);
            draw_square(dl, viewport, joint->groundAnchorB, GROUND_COLOR);
            draw_link(dl, viewport, joint->groundAnchorA, joint->groundAnchorB, GROUND_COLOR, 1.5f);

            if(transformA){
                Vector2f worldA = local_to_world(*transformA, joint->localAnchorA);
                draw_anchor(dl, viewport, worldA, ANCHOR_A_COLOR, MAIN_ANCHOR_RADIUS);
                draw_link(dl, viewport, joint->groundAnchorA, worldA, LINK_COLOR);
            }

            if(transformB){
                Vector2f worldB = local_to_world(*transformB, joint->localAnchorB);
                draw_anchor(dl, viewport, worldB, ANCHOR_B_COLOR);
                draw_link(dl, viewport, joint->groundAnchorB, worldB, LINK_COLOR);
            }
        } else if(auto* joint = selected.try_get_component<MotorJointComponent>()){
            const TransformComponent* transformA = body_transform(joint->entityA);
            const TransformComponent* transformB = body_transform(joint->entityB);

            if(transformA){
                draw_anchor(dl, viewport, transformA->position, ANCHOR_A_COLOR, MAIN_ANCHOR_RADIUS);

                Vector2f target = local_to_world(*transformA, joint->linearOffset);
                draw_cross(dl, viewport, target, TARGET_COLOR);
                draw_link(dl, viewport, transformA->position, target, LINK_COLOR);

                if(transformB)
                    draw_link(dl, viewport, transformB->position, target, LIMIT_COLOR, 1.5f);
            }

            if(transformB)
                draw_anchor(dl, viewport, transformB->position, ANCHOR_B_COLOR);
        } else if(auto* joint = selected.try_get_component<MouseJointComponent>()){
            const TransformComponent* transformB = body_transform(joint->entityB);

            draw_cross(dl, viewport, joint->target, TARGET_COLOR);

            if(transformB)
                draw_link(dl, viewport, transformB->position, joint->target, LINK_COLOR);
        }

        ImGui::End();
    }
}
