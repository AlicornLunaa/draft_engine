#include "draft/phys/collider.hpp"
#include "draft/phys/fixture_def.hpp"
#include "draft/phys/shapes/polygon_shape.hpp"
#include <cassert>

namespace Draft {
    // Private functions
    void Collider::copy_collider(const Collider& other){
        if(is_attached()){
            detach();
        }

        rigidBodyPtr = nullptr;
        position = other.position;
        origin = other.origin;
        scale = other.scale;
        rotation = other.rotation;

        for(auto& shape : other.shapes){
            shapes.push_back(shape->clone());
        }
    }

    // Constructors
    Collider::Collider(const JSON& json){
        // JSON loader for collider
        for(JSON shapeData : json){
            PolygonShape shape;
            JSON indices = shapeData["indices"];
            JSON vertices = shapeData["vertices"];
            
            for(int index : indices){
                float x = vertices[index * 2];
                float y = vertices[index * 2 + 1];
                shape.add_vertex({x, y});
            }

            shape.restitution = shapeData["restitution"];
            shape.density = shapeData["density"];
            shape.friction = shapeData["friction"];
            shape.isSensor = shapeData["sensor"];
            shape.isConvex = shapeData["convex"];

            add_shape(shape);
        }
    }

    Collider::Collider(const Collider& other){
        copy_collider(other);
    }

    // Operators
    Collider& Collider::operator=(const Collider& other){
        if(this != &other)
            copy_collider(other);

        return *this;
    }

    // Functions
    Shape* Collider::add_shape(const Shape& shape){
        // Copy and add shape to this collider
        shapes.push_back(shape.clone());

        if(is_attached())
            update_collider();
        
        return shapes.back().get();
    }

    void Collider::del_shape(const Shape* shapePtr){
        // Removes the shape
        for(size_t i = 0; i < shapes.size(); i++){
            if(shapes[i].get() == shapePtr){
                shapes.erase(shapes.begin() + i);
                break;
            }
        }

        if(is_attached())
            update_collider();
    }

    void Collider::clear(){
        // Remove every shape
        shapes.clear();

        if(is_attached())
            update_collider();
    }

    void Collider::set_position(const Vector2f& position){ this->position = position; }
    void Collider::set_origin(const Vector2f& origin){ this->origin = origin; }
    void Collider::set_scale(const Vector2f& scale){ this->scale = scale; }
    void Collider::set_rotation(float rotation){ this->rotation = rotation; }

    const Vector2f& Collider::get_position() const { return position; }
    const Vector2f& Collider::get_origin() const { return origin; }
    const Vector2f& Collider::get_scale() const { return scale; }
    float Collider::get_rotation() const { return rotation; }

    void Collider::attach(RigidBody* rigidBodyPtr){
        assert(rigidBodyPtr && "rigidBodyPtr cannot be null");

        // Remove previous attachment, it can only attach to one at a time!
        if(is_attached())
            detach();

        // Save pointer and attach each shape as fixtures
        this->rigidBodyPtr = rigidBodyPtr;

        for(auto& shapePtr : shapes){
            FixtureDef fixtureDef;
            fixtureDef.density = shapePtr->density;
            fixtureDef.friction = shapePtr->friction;
            fixtureDef.isSensor = shapePtr->isSensor;
            fixtureDef.restitution = shapePtr->restitution;
            fixtureDef.shape = shapePtr.get();

            Fixture* fixturePtr = rigidBodyPtr->create_fixture(fixtureDef);
            fixtures.push_back(fixturePtr);
        }
    }

    void Collider::update_collider(){
        assert(rigidBodyPtr && "Must have a rigid body already attached");
        attach(rigidBodyPtr);
    }

    void Collider::detach(){
        assert(rigidBodyPtr && "Cannot detach from nothing");

        for(Fixture* fixturePtr : fixtures)
            rigidBodyPtr->destroy_fixture(fixturePtr);

        rigidBodyPtr = nullptr;
        fixtures.clear();
    }
}