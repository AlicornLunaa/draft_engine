#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/core/engine.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/util/files/file_handle.hpp"

namespace Draft {
    /**
     * @brief Writes every system attached to @p scene (in attach order, alongside each system's
     * own reflected data) and every live entity's registered component data to @p file as JSON.
     *
     * Only system/component types actually registered in @p engine's catalogs round-trip. A
     * system attached directly via SystemRegistry::add<T>()/emplace<T>() (bypassing
     * SystemCatalog), or a component with no matching ComponentCatalog entry, is silently
     * skipped, same as by_name()/by_type() returning nullptr anywhere else in the catalogs.
     */
    void save_scene(const Scene& scene, const Engine& engine, AssetManager& assets, const FileHandle& file);

    /**
     * @brief Reads a scene previously written by save_scene() into @p scene. @p scene is meant to
     * be freshly constructed, entities/components are only ever added, never cleared first.
     *
     * Systems are attached first (via each SystemTypeInterface::add(), constructing through the
     * type's registered factory, then deserializing its own reflected data back onto the
     * attached instance), before any entity or component exists. That way, anything a system
     * reacts to entities or components being created for is already wired up for every entity
     * load creates from here on, including PhysicsSystem's on_construct<RigidBodyComponent>/
     * on_construct<ColliderComponent> hooks, which is what materializes the corresponding live
     * box2d body/fixture with no explicit physics code in this function.
     *
     * Entities are created next, all at once, before any component is added, so a component's
     * cross-reference to another entity (e.g. ChildComponent::parent) can resolve regardless of
     * which entity in the file defines it, or whether it appears earlier or later in the file.
     */
    void load_scene(Scene& scene, const Engine& engine, AssetManager& assets, const FileHandle& file);
}
