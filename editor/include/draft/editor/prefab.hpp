#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/math/glm.hpp"

#include <filesystem>

namespace Draft {
    class EditorApplication;

    /**
     * @brief Saves an entity subtree to a JSON ".prefab" asset
     */
    class PrefabManager {
    public:
        explicit PrefabManager(EditorApplication& app);

        /**
         * @brief Serializes @p root and every descendant (walked via ParentComponent::children)
         * to @p file. A component field referencing an entity outside the subtree (e.g. a
         * joint's other body) serializes as an unset Entity rather than throwing, so saving a
         * partial subtree never crashes, it just drops that one link.
         */
        void save_prefab(Entity root, const std::filesystem::path& file) const;

        /**
         * @brief Instantiates @p file's subtree into gameScene. Every instantiated entity with a
         * TransformComponent is shifted by the same offset, chosen so the subtree root lands at
         * @p worldPosition, preserving the rest of the subtree's relative layout. The root never
         * has a ChildComponent (save_prefab() drops it), so it comes in as a new root entity.
         * @return The new root entity, or an invalid Entity if @p file doesn't exist.
         */
        Entity instantiate_prefab(const std::filesystem::path& file, const Vector2f& worldPosition) const;

    private:
        EditorApplication& m_app;
    };
}
