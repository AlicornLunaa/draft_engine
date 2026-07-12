#include "draft/phys/collider.hpp"
#include "draft/math/glm.hpp"
#include "draft/phys/fixture_def.hpp"
#include "draft/phys/shapes/chain_shape.hpp"
#include "draft/phys/shapes/circle_shape.hpp"
#include "draft/phys/shapes/edge_shape.hpp"
#include "draft/phys/shapes/polygon_shape.hpp"
#include <algorithm>
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

    void Collider::set_new_transform(Vector2f newPosition, Vector2f newOrigin, Vector2f newScale, float newRotation){
        // Move all the points back to the original position
        Matrix4 worldToLocal = Math::inverse(Optimal::fast_model_matrix(position, rotation, scale, origin, 0));
        Matrix4 localToNewWorld = Optimal::fast_model_matrix(newPosition, newRotation, newScale, newOrigin, 0);
        Matrix4 transform = localToNewWorld * worldToLocal;

        for(auto& shape : shapes){
            switch(shape->type){
                case ShapeType::POLYGON: {
                    PolygonShape* polygon = static_cast<PolygonShape*>(shape.get());

                    for(size_t i = 0; i < polygon->get_vertex_count(); i++)
                        polygon->set_vertex(i, transform * Vector4f(polygon->get_vertex(i), 0, 1));

                    break;
                }

                case ShapeType::CIRCLE: {
                    CircleShape* circle = static_cast<CircleShape*>(shape.get());
                    circle->set_position(transform * Vector4f(circle->get_position(), 0, 1));
                    circle->set_radius(circle->get_radius() / Math::length(scale) * Math::length(newScale));
                    break;
                }

                case ShapeType::EDGE: {
                    EdgeShape* edge = static_cast<EdgeShape*>(shape.get());
                    edge->set_start(transform * Vector4f(edge->get_start(), 0, 1));
                    edge->set_end(transform * Vector4f(edge->get_end(), 0, 1));
                    break;
                }

                case ShapeType::CHAIN: {
                    ChainShape* chain = static_cast<ChainShape*>(shape.get());
                    auto points = chain->get_points();
                    chain->clear();

                    for(auto& p : points){
                        chain->add(transform * Vector4f(p, 0, 1));
                    }

                    break;
                }
            }
        }

        // Save new data
        position = newPosition;
        origin = newOrigin;
        scale = newScale;
        rotation = newRotation;

        if(is_attached())
            update_collider();
    }

    void Collider::invalidate(){
        rigidBodyPtr = nullptr;
        fixtures.clear();
    }

    // Constructors
    Collider::Collider(const Collider& other){
        copy_collider(other);
    }

    Collider::~Collider(){
        // Collider destroyed while still attached must destroy its box2d fixtures
        // (and unregister itself from the body) first, otherwise those fixtures leak in the live b2Body
        if(is_attached())
            detach();
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

    void Collider::set_position(const Vector2f& position){ set_new_transform(position, origin, scale, rotation); }
    void Collider::set_origin(const Vector2f& origin){ set_new_transform(position, origin, scale, rotation); }
    void Collider::set_scale(const Vector2f& scale){ set_new_transform(position, origin, scale, rotation); }
    void Collider::set_rotation(float rotation){ set_new_transform(position, origin, scale, rotation); }

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
        rigidBodyPtr->attachedColliders.push_back(this);

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

        auto& attached = rigidBodyPtr->attachedColliders;
        attached.erase(std::remove(attached.begin(), attached.end(), this), attached.end());

        rigidBodyPtr = nullptr;
        fixtures.clear();
    }

    bool Collider::test_point(Vector2f point) const {
        // Test all the points in the shape
        for(auto& shape : shapes){
            if(shape->contains(point)){
                return true;
            }
        }

        return false;
    }
}
