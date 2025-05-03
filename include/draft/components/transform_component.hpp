#pragma once

#include "draft/components/rigid_body_component.hpp"
#include "draft/core/entity.hpp"
#include "draft/math/glm.hpp"
#include <cassert>

namespace Draft {
    struct TransformComponent {
        // Variables
        Vector2f position = {};
        float rotation = 0.f;

        // Functions
        Matrix3 get_transform() const {
            Matrix3 mat = Matrix3(1.f);
            mat = Math::translate(mat, position);
            mat = Math::rotate(mat, rotation);
            return mat;
        }

        Matrix4 get_matrix() const {
            Matrix4 mat = Matrix4(1.f);
            mat = Math::translate(mat, {position, 0.f});
            mat = Math::rotate(mat, rotation, {0, 0, 1});
            return mat;
        }

        // Operators
        operator Matrix4 () {
            return get_matrix();
        }

        // Functions
        void force_sync(Draft::Entity entity){
            // Forces the rigidbody to sync
            assert(entity.try_get_component<TransformComponent>() == this && "This entity is not the owner of this transform");

            if(auto* ptr = entity.try_get_component<NativeBodyComponent>()){
                ptr->bodyPtr->set_transform(position, rotation);
                ptr->deltaP = position;
                ptr->deltaR = rotation;
            }
        }
    };

    typedef TransformComponent Transform;
}