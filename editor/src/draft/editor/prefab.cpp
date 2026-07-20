#include "draft/editor/prefab.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/serializer.hpp" // IWYU pragma: keep

#include <cstdint>
#include <typeindex>

namespace Draft {
    namespace {
        void collect_subtree(Entity entity, std::vector<Entity>& out){
            out.push_back(entity);

            if(auto* parentComp = entity.try_get_component<ParentComponent>())
                for(Entity child : parentComp->children)
                    collect_subtree(child, out);
        }
    }

    PrefabManager::PrefabManager(EditorApplication& app) : m_app(app) {}

    void PrefabManager::save_prefab(Entity root, const std::filesystem::path& file) const {
        if(!root.is_valid())
            return;

        std::vector<Entity> subtree;
        collect_subtree(root, subtree);

        SceneSerializationContext ctx;
        ctx.assets = &m_app.assets;

        // Every live entity defaults to the same "unset" id Entity::serialize() already writes
        // for a null Entity field, so a reference to something outside the subtree resolves to
        // that instead of entityToId.at() throwing on a key nothing below ever assigns.
        Registry& registry = m_app.gameScene.get_registry();
        for(entt::entity raw : registry.storage<entt::entity>())
            if(registry.valid(raw))
                ctx.entityToId[raw] = UINT32_MAX;

        for(std::size_t i = 0; i < subtree.size(); i++)
            ctx.entityToId[static_cast<entt::entity>(subtree[i])] = static_cast<uint32_t>(i);

        Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

        std::type_index childType(typeid(ChildComponent));
        JSON entitiesJson = JSON::array();

        for(Entity entity : subtree){
            JSON entityJson = JSON::object();

            for(ComponentTypeInterface* entry : m_app.gameEngine.components().all()){
                // The root's own ChildComponent points at a parent outside the prefab; dropping
                // it here is what makes an instantiated copy a root instead of a dangling child.
                if(entity == root && entry->type() == childType)
                    continue;

                if(!entry->has(entity))
                    continue;

                JSON data = JSON::object();
                entry->serialize(entity, data);
                entityJson[entry->name()] = data;
            }

            entitiesJson.push_back(entityJson);
        }

        JSON json = JSON::object();
        json["entities"] = entitiesJson;

        HostFileSystem().create_directories(file);
        HostFileSystem().open(file).write_string(json.dump(4));
    }

    Entity PrefabManager::instantiate_prefab(const std::filesystem::path& file, const Vector2f& worldPosition) const {
        FileHandle handle = HostFileSystem().open(file);
        if(!handle.exists())
            return Entity();

        JSON json(handle);
        JSON& entitiesJson = json.at("entities");

        SceneSerializationContext ctx;
        ctx.assets = &m_app.assets;
        ctx.idToEntity.reserve(entitiesJson.size());

        Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

        // Same two-pass shape as load_scene(): every entity exists before any component is
        // added, so a cross-entity reference resolves regardless of file order.
        for(std::size_t i = 0; i < entitiesJson.size(); i++)
            ctx.idToEntity.push_back(m_app.gameScene.create_entity());

        for(std::size_t i = 0; i < entitiesJson.size(); i++){
            Entity entity = ctx.idToEntity[i];
            JSON& entityJson = entitiesJson.at(i);

            for(ComponentTypeInterface* entry : m_app.gameEngine.components().all()){
                if(!entityJson.contains(entry->name()))
                    continue;

                entry->deserialize(entity, entityJson.at(entry->name()));
            }
        }

        if(ctx.idToEntity.empty())
            return Entity();

        // save_prefab() always writes the subtree root first, see collect_subtree().
        Entity root = ctx.idToEntity[0];

        Vector2f offset = worldPosition;
        if(auto* rootTransform = root.try_get_component<TransformComponent>())
            offset = worldPosition - rootTransform->position;

        for(Entity entity : ctx.idToEntity)
            if(auto* transform = entity.try_get_component<TransformComponent>())
                transform->position += offset;

        return root;
    }
}
