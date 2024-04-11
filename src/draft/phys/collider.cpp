#include "draft/phys/collider.hpp"

namespace Draft {
    // Constructors
    Collider::Collider(){
    }

    Collider::Collider(const Collider& other){
    }

    Collider::~Collider(){
        for(const auto* ptr : shapes){
            delete ptr;
        }
    }

    // Functions
}