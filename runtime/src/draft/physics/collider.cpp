#include "draft/physics/collider.hpp"
#include "draft/math/glm.hpp"
#include "draft/physics/fixture_def.hpp"
#include "draft/physics/shapes/chain_shape.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/shapes/edge_shape.hpp"
#include "draft/physics/shapes/polygon_shape.hpp"
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
        // json[key] returns a base nlohmann::json&, and assigning a Draft::JSON into that by
        // value needs the explicit cast below, plain std::move() picks the wrong constructor.
        auto put = [](JSON& obj, const char* key, const auto& value){
            JSON child;
            Serializer::serialize(value, child);
            obj[key] = static_cast<nlohmann::json&&>(child);
        };

        JSON shapesJson = JSON::array();

        for(auto& shape : collider.shapes){
            JSON shapeJson;
            put(shapeJson, "type", shape->type);
            put(shapeJson, "friction", shape->friction);
            put(shapeJson, "restitution", shape->restitution);
            put(shapeJson, "density", shape->density);
            put(shapeJson, "isSensor", shape->isSensor);
            put(shapeJson, "isConvex", shape->isConvex);

            switch(shape->type){
                case ShapeType::CIRCLE: {
                    auto* circle = static_cast<CircleShape*>(shape.get());
                    put(shapeJson, "position", circle->get_position());
                    put(shapeJson, "radius", circle->get_radius());
                    break;
                }

                case ShapeType::POLYGON: {
                    auto* polygon = static_cast<PolygonShape*>(shape.get());
                    put(shapeJson, "vertices", polygon->get_vertices());
                    break;
                }

                case ShapeType::EDGE: {
                    auto* edge = static_cast<EdgeShape*>(shape.get());
                    put(shapeJson, "start", edge->get_start());
                    put(shapeJson, "end", edge->get_end());
                    break;
                }

                case ShapeType::CHAIN: {
                    auto* chain = static_cast<ChainShape*>(shape.get());
                    put(shapeJson, "chainType", chain->get_chain_type());
                    put(shapeJson, "points", chain->get_points());
                    put(shapeJson, "previous", chain->get_previous());
                    put(shapeJson, "next", chain->get_next());
                    break;
                }
            }

            shapesJson.push_back(std::move(shapeJson));
        }

        json["shapes"] = static_cast<nlohmann::json&&>(shapesJson);
        put(json, "position", collider.position);
        put(json, "origin", collider.origin);
        put(json, "scale", collider.scale);
        put(json, "rotation", collider.rotation);
    }

    void Collider::deserialize(Collider& collider, JSON& json){
        collider.clear();

        // json.at(key) returns a base nlohmann::json&, which can't bind to a JSON& parameter,
        // so copy into a real JSON first (same reasoning as the reflect tier's JSON deserialize).
        auto get = [](JSON& obj, const char* key, auto& value){
            JSON child = obj.at(key);
            Serializer::deserialize(value, child);
        };

        JSON shapesJson = json.at("shapes");

        for(JSON shapeJson : shapesJson){
            ShapeType type{};
            float friction = 0.f, restitution = 0.f, density = 0.f;
            bool isSensor = false, isConvex = true;

            get(shapeJson, "type", type);
            get(shapeJson, "friction", friction);
            get(shapeJson, "restitution", restitution);
            get(shapeJson, "density", density);
            get(shapeJson, "isSensor", isSensor);
            get(shapeJson, "isConvex", isConvex);

            Shape* added = nullptr;

            switch(type){
                case ShapeType::CIRCLE: {
                    CircleShape shape;
                    Vector2f position{};
                    float radius = 0.f;
                    get(shapeJson, "position", position);
                    get(shapeJson, "radius", radius);
                    shape.set_position(position);
                    shape.set_radius(radius);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::POLYGON: {
                    PolygonShape shape;
                    std::vector<Vector2f> vertices;
                    get(shapeJson, "vertices", vertices);
                    for(auto& vertex : vertices)
                        shape.add_vertex(vertex);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::EDGE: {
                    Vector2f start{}, end{};
                    get(shapeJson, "start", start);
                    get(shapeJson, "end", end);
                    EdgeShape shape(start, end);
                    added = collider.add_shape(shape);
                    break;
                }

                case ShapeType::CHAIN: {
                    ChainShape::ChainType chainType{};
                    get(shapeJson, "chainType", chainType);

                    std::vector<Vector2f> points;
                    Vector2f previous{}, next{};
                    get(shapeJson, "points", points);
                    get(shapeJson, "previous", previous);
                    get(shapeJson, "next", next);

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

        get(json, "position", collider.position);
        get(json, "origin", collider.origin);
        get(json, "scale", collider.scale);
        get(json, "rotation", collider.rotation);
    }
}
