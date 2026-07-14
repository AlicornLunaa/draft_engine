#include "draft/ecs/scene_serializer.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/ecs/system_catalog.hpp"
#include "draft/util/serialization/context.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace Draft {
    void save_scene(const Scene& scene, const Engine& engine, AssetManager& assets, const FileHandle& file){
        SceneSerializationContext ctx;
        ctx.assets = &assets;

        // Pass 1: assign every live entity a sequential id, in iteration order. Kept as its own
        // vector (rather than re-deriving order from ctx.entityToId, an unordered_map) so pass 3
        // below writes entities back out in that same order.
        std::vector<entt::entity> orderedEntities;
        if(const auto* entityStorage = scene.get_registry().storage<entt::entity>()){
            for(entt::entity raw : *entityStorage){
                ctx.entityToId[raw] = static_cast<uint32_t>(orderedEntities.size());
                orderedEntities.push_back(raw);
            }
        }

        // Entity/Resource<T>'s tier-1 (de)serialization reaches into this for the rest of the call.
        Serializer::ScopedContext<SceneSerializationContext> scope(ctx);
        JSON json = JSON::object();

        // Pass 2: which systems are attached, in attach order, each with its own reflected data.
        JSON systemsJson = JSON::array();
        for(const std::type_index& type : scene.get_systems().registered_types()){
            SystemTypeInterface* entry = engine.systems().by_type(type);

            if(!entry)
                continue;

            JSON data = JSON::object();
            entry->serialize(scene.get_systems(), data);
            systemsJson.push_back(JSON::object({{"name", entry->name()}, {"data", data}}));
        }
        json["systems"] = systemsJson;

        // Pass 3: per-entity component data, in the order ids were assigned above.
        JSON entitiesJson = JSON::array();
        for(entt::entity raw : orderedEntities){
            // save_scene only ever reads through this handle. Entity has no const-aware API of
            // its own (see entity.hpp), so a mutable Scene* is needed to construct one at all.
            Entity entity(const_cast<Scene*>(&scene), raw);

            JSON entityJson = JSON::object();
            for(ComponentTypeInterface* entry : engine.components().all()){
                if(!entry->has(entity))
                    continue;

                JSON data = JSON::object();
                entry->serialize(entity, data);
                entityJson[entry->name()] = data;
            }

            entitiesJson.push_back(entityJson);
        }
        json["entities"] = entitiesJson;

        file.write_string(json.dump(4));
    }

    void load_scene(Scene& scene, const Engine& engine, AssetManager& assets, const FileHandle& file){
        JSON json = JSON(file);

        SceneSerializationContext ctx;
        ctx.assets = &assets;

        JSON& entitiesJson = json.at("entities");
        ctx.idToEntity.reserve(entitiesJson.size());

        // Entity/Resource<T>'s tier-1 (de)serialization reaches into this for the rest of the
        // call, active from before pass 1 in case a system's own data references either.
        Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

        // Load pass 1: attach systems via their registered factory, in file (original attach)
        // order, then restore each one's own reflected data onto the instance just attached.
        JSON& systemsJson = json.at("systems");
        for(std::size_t i = 0; i < systemsJson.size(); i++){
            JSON& systemJson = systemsJson.at(i);
            std::string name = systemJson.at("name").get<std::string>();

            SystemTypeInterface* entry = engine.systems().by_name(name);
            if(!entry)
                continue;

            entry->add(scene);
            entry->deserialize(scene.get_systems(), systemJson.at("data"));
        }

        // Load pass 2: create every saved entity up front, in file order, so a cross-reference
        // to an entity defined later in the file (or to one appearing earlier) already resolves
        // once pass 3 runs.
        for(std::size_t i = 0; i < entitiesJson.size(); i++)
            ctx.idToEntity.push_back(scene.create_entity());

        // Load pass 3: restore each entity's component data, now that every system a component's
        // reactive hook might depend on (e.g. PhysicsSystem) is already attached.
        for(std::size_t i = 0; i < entitiesJson.size(); i++){
            Entity entity = ctx.idToEntity[i];
            JSON& entityJson = entitiesJson.at(i);

            for(ComponentTypeInterface* entry : engine.components().all()){
                if(!entityJson.contains(entry->name()))
                    continue;

                entry->deserialize(entity, entityJson.at(entry->name()));
            }
        }
    }
}
