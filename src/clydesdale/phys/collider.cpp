#include "collider.hpp"

namespace Clydesdale {
    // Constructors
    Collider::Collider(){
    }

    Collider::~Collider(){
        for(const auto* ptr : shapes){
            delete ptr;
        }
    }

    // Functions
}