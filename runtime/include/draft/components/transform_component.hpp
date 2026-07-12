#pragma once

#include "draft/components/rigid_body_component.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/math/glm.hpp"
#include "draft/util/reflectable.hpp"

#include <cassert>

namespace Draft {
    /**
     * @brief 2D position + rotation.
     */
    struct TransformComponent {
        DRAFT_REFLECTED(Vector2f, position) = {};
        DRAFT_REFLECTED(float, rotation) = 0.f;

        /**
         * @brief This transform as a 2D (3x3) matrix.
         */
        Matrix3 get_transform() const {
            Matrix3 mat = Matrix3(1.f);
            mat = Math::translate(mat, position);
            mat = Math::rotate(mat, rotation);
            return mat;
        }

        /**
         * @brief This transform as a 3D (4x4) matrix, for use alongside 3D rendering/camera math.
         */
        Matrix4 get_matrix() const {
            Matrix4 mat = Matrix4(1.f);
            mat = Math::translate(mat, {position, 0.f});
            mat = Math::rotate(mat, rotation, {0, 0, 1});
            return mat;
        }

        operator Matrix4() const {
            return get_matrix();
        }

        /**
         * @brief Forces the entity's `NativeBodyComponent` (if any) to adopt this transform's
         * current `position`/`rotation` immediately
         */
        void force_sync(Entity entity){
            assert(entity.try_get_component<TransformComponent>() == this && "This entity is not the owner of this transform");

            if(auto* ptr = entity.try_get_component<NativeBodyComponent>()){
                ptr->bodyPtr->set_transform(position, rotation);
                ptr->deltaP = position;
                ptr->deltaR = rotation;
            }
        }

        DRAFT_REFLECTABLE(TransformComponent, position, rotation)
    };

    using Transform = TransformComponent;
}
