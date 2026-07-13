#include "draft/physics/collider.hpp"
#include "draft/math/glm.hpp"
#include "draft/physics/fixture_def.hpp"
#include "draft/physics/shapes/chain_shape.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/shapes/edge_shape.hpp"
#include "draft/physics/shapes/polygon_shape.hpp"
#include "draft/util/serialization/serializer.hpp"
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

    // Serialization
    // Binary encoding is shape count, then per shape a type tag, the shared Shape fields, and its
    // own geometry, then the collider's own transform. Variable length, so a Collider inside a
    // larger byte stream needs to advance its own span rather than assume sizeof(Collider).
    void Collider::serialize(const Collider& collider, Binary::ByteArray& out){
        Serializer::serialize(collider.shapes.size(), out);

        for(auto& shape : collider.shapes){
            Serializer::serialize(shape->type, out);
            Serializer::serialize(shape->friction, out);
            Serializer::serialize(shape->restitution, out);
            Serializer::serialize(shape->density, out);
            Serializer::serialize(shape->isSensor, out);
            Serializer::serialize(shape->isConvex, out);

            switch(shape->type){
                case ShapeType::CIRCLE: {
                    auto* circle = static_cast<CircleShape*>(shape.get());
                    Serializer::serialize(circle->get_position(), out);
                    Serializer::serialize(circle->get_radius(), out);
                    break;
                }

                case ShapeType::POLYGON: {
                    auto* polygon = static_cast<PolygonShape*>(shape.get());
                    Serializer::serialize(polygon->get_vertices(), out);
                    break;
                }

                case ShapeType::EDGE: {
                    auto* edge = static_cast<EdgeShape*>(shape.get());
                    Serializer::serialize(edge->get_start(), out);
                    Serializer::serialize(edge->get_end(), out);
                    break;
                }

                case ShapeType::CHAIN: {
                    auto* chain = static_cast<ChainShape*>(shape.get());
                    Serializer::serialize(chain->get_chain_type(), out);
                    Serializer::serialize(chain->get_points(), out);
                    Serializer::serialize(chain->get_previous(), out);
                    Serializer::serialize(chain->get_next(), out);
                    break;
                }
            }
        }

        Serializer::serialize(collider.position, out);
        Serializer::serialize(collider.origin, out);
        Serializer::serialize(collider.scale, out);
        Serializer::serialize(collider.rotation, out);
    }

    void Collider::deserialize(Collider& collider, Binary::ByteView span){
        collider.clear();

        size_t shapeCount = 0;
        Serializer::deserialize_and_advance(shapeCount, span);

        for(size_t i = 0; i < shapeCount; i++){
            ShapeType type{};
            float friction = 0.f, restitution = 0.f, density = 0.f;
            bool isSensor = false, isConvex = true;

            Serializer::deserialize_and_advance(type, span);
            Serializer::deserialize_and_advance(friction, span);
            Serializer::deserialize_and_advance(restitution, span);
            Serializer::deserialize_and_advance(density, span);
            Serializer::deserialize_and_advance(isSensor, span);
            Serializer::deserialize_and_advance(isConvex, span);

            Shape* added = nullptr;

            switch(type){
                case ShapeType::CIRCLE: {
                    CircleShape shape;
                    Vector2f position{};
                    float radius = 0.f;
                    Serializer::deserialize_and_advance(position, span);
                    Serializer::deserialize_and_advance(radius, span);
                    shape.set_position(position);
                    shape.set_radius(radius);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::POLYGON: {
                    PolygonShape shape;
                    std::vector<Vector2f> vertices;
                    Serializer::deserialize_and_advance(vertices, span);
                    for(auto& vertex : vertices)
                        shape.add_vertex(vertex);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::EDGE: {
                    Vector2f start{}, end{};
                    Serializer::deserialize_and_advance(start, span);
                    Serializer::deserialize_and_advance(end, span);
                    EdgeShape shape(start, end);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::CHAIN: {
                    ChainShape::ChainType chainType{};
                    Serializer::deserialize_and_advance(chainType, span);

                    std::vector<Vector2f> points;
                    Vector2f previous{}, next{};
                    Serializer::deserialize_and_advance(points, span);
                    Serializer::deserialize_and_advance(previous, span);
                    Serializer::deserialize_and_advance(next, span);

                    ChainShape shape(chainType);
                    for(auto& point : points)
                        shape.add(point);
                    shape.set_previous(previous);
                    shape.set_next(next);

                    added = collider.add_shape(shape);
                    break;
                }
            }

            if(added){
                added->friction = friction;
                added->restitution = restitution;
                added->density = density;
                added->isSensor = isSensor;
                added->isConvex = isConvex;
            }
        }

        Serializer::deserialize_and_advance(collider.position, span);
        Serializer::deserialize_and_advance(collider.origin, span);
        Serializer::deserialize_and_advance(collider.scale, span);
        Serializer::deserialize_and_advance(collider.rotation, span);
    }

    void Collider::serialize(const Collider& collider, JSON& json){
        JSON shapesJson = JSON::array();

        for(auto& shape : collider.shapes){
            JSON shapeJson;
            Serializer::serialize(shape->type, shapeJson["type"]);
            Serializer::serialize(shape->friction, shapeJson["friction"]);
            Serializer::serialize(shape->restitution, shapeJson["restitution"]);
            Serializer::serialize(shape->density, shapeJson["density"]);
            Serializer::serialize(shape->isSensor, shapeJson["isSensor"]);
            Serializer::serialize(shape->isConvex, shapeJson["isConvex"]);

            switch(shape->type){
                case ShapeType::CIRCLE: {
                    auto* circle = static_cast<CircleShape*>(shape.get());
                    Serializer::serialize(circle->get_position(), shapeJson["position"]);
                    Serializer::serialize(circle->get_radius(), shapeJson["radius"]);
                    break;
                }

                case ShapeType::POLYGON: {
                    auto* polygon = static_cast<PolygonShape*>(shape.get());
                    Serializer::serialize(polygon->get_vertices(), shapeJson["vertices"]);
                    break;
                }

                case ShapeType::EDGE: {
                    auto* edge = static_cast<EdgeShape*>(shape.get());
                    Serializer::serialize(edge->get_start(), shapeJson["start"]);
                    Serializer::serialize(edge->get_end(), shapeJson["end"]);
                    break;
                }

                case ShapeType::CHAIN: {
                    auto* chain = static_cast<ChainShape*>(shape.get());
                    Serializer::serialize(chain->get_chain_type(), shapeJson["chainType"]);
                    Serializer::serialize(chain->get_points(), shapeJson["points"]);
                    Serializer::serialize(chain->get_previous(), shapeJson["previous"]);
                    Serializer::serialize(chain->get_next(), shapeJson["next"]);
                    break;
                }
            }

            shapesJson.push_back(std::move(shapeJson));
        }

        json["shapes"] = std::move(shapesJson);
        Serializer::serialize(collider.position, json["position"]);
        Serializer::serialize(collider.origin, json["origin"]);
        Serializer::serialize(collider.scale, json["scale"]);
        Serializer::serialize(collider.rotation, json["rotation"]);
    }

    void Collider::deserialize(Collider& collider, const JSON& json){
        collider.clear();

        const JSON& shapesJson = json.at("shapes");

        for(JSON shapeJson : shapesJson){
            ShapeType type{};
            float friction = 0.f, restitution = 0.f, density = 0.f;
            bool isSensor = false, isConvex = true;

            Serializer::deserialize(type, shapeJson.at("type"));
            Serializer::deserialize(friction, shapeJson.at("friction"));
            Serializer::deserialize(restitution, shapeJson.at("restitution"));
            Serializer::deserialize(density, shapeJson.at("density"));
            Serializer::deserialize(isSensor, shapeJson.at("isSensor"));
            Serializer::deserialize(isConvex, shapeJson.at("isConvex"));

            Shape* added = nullptr;

            switch(type){
                case ShapeType::CIRCLE: {
                    CircleShape shape;
                    Vector2f position{};
                    float radius = 0.f;
                    Serializer::deserialize(position, shapeJson.at("position"));
                    Serializer::deserialize(radius, shapeJson.at("radius"));
                    shape.set_position(position);
                    shape.set_radius(radius);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::POLYGON: {
                    PolygonShape shape;
                    std::vector<Vector2f> vertices;
                    Serializer::deserialize(vertices, shapeJson.at("vertices"));
                    for(auto& vertex : vertices)
                        shape.add_vertex(vertex);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::EDGE: {
                    Vector2f start{}, end{};
                    Serializer::deserialize(start, shapeJson.at("start"));
                    Serializer::deserialize(end, shapeJson.at("end"));
                    EdgeShape shape(start, end);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::CHAIN: {
                    ChainShape::ChainType chainType{};
                    Serializer::deserialize(chainType, shapeJson.at("chainType"));

                    std::vector<Vector2f> points;
                    Vector2f previous{}, next{};
                    Serializer::deserialize(points, shapeJson.at("points"));
                    Serializer::deserialize(previous, shapeJson.at("previous"));
                    Serializer::deserialize(next, shapeJson.at("next"));

                    ChainShape shape(chainType);
                    for(auto& point : points)
                        shape.add(point);
                    shape.set_previous(previous);
                    shape.set_next(next);

                    added = collider.add_shape(shape);
                    break;
                }
            }

            if(added){
                added->friction = friction;
                added->restitution = restitution;
                added->density = density;
                added->isSensor = isSensor;
                added->isConvex = isConvex;
            }
        }

        Serializer::deserialize(collider.position, json.at("position"));
        Serializer::deserialize(collider.origin, json.at("origin"));
        Serializer::deserialize(collider.scale, json.at("scale"));
        Serializer::deserialize(collider.rotation, json.at("rotation"));
    }
}
