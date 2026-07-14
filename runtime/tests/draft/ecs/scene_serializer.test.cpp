#include <gtest/gtest.h>
#include "draft/ecs/scene_serializer.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/components/rigid_body_component.hpp"
#include "draft/components/collider_component.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/components/transform_component.hpp"
#include "draft/core/engine.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/physics_system.hpp"
#include "draft/ecs/relationship_components.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/ecs/system.hpp"
#include "draft/physics/shapes/circle_shape.hpp"
#include "draft/physics/world.hpp"
#include "draft/util/files/disk_file_provider.hpp"

#include <memory>

using namespace Draft;

namespace {
    // A system holding its own reflected, savable data alongside its per-tick behavior, the
    // same shape as the real AudioSystem::dopplerSensitivity. Kept independent of physics or
    // rendering setup, per this file's testing convention (see component_catalog.test.cpp/system_catalog.test.cpp).
    struct GravitySystem : AbstractSystem {
        DRAFT_REFLECTED(float, strength) = 9.8f;

        void update(Time) override {}

        DRAFT_REFLECTABLE(GravitySystem, strength)
    };

    // Finds the live entity carrying a TagComponent equal to @p tag, or an invalid Entity if none does.
    Entity find_by_tag(Scene& scene, const std::string& tag){
        for(entt::entity raw : scene.get_registry().storage<entt::entity>()){
            Entity e(&scene, raw);
            if(e.has_component<TagComponent>() && e.get_component<TagComponent>().tag == tag)
                return e;
        }

        return Entity();
    }
}

TEST(SceneSerializer, RoundTripsEntitiesComponentsAndSystemData)
{
    Engine engine;
    engine.systems().register_system<GravitySystem>([](Scene&){ return std::make_unique<GravitySystem>(); });

    AssetManager assets;

    Scene scene;
    scene.get_systems().add<GravitySystem>().strength = 12.5f;

    Entity parent = scene.create_entity();
    parent.add_component<TagComponent>(TagComponent{"Parent"});
    parent.add_component<TransformComponent>(TransformComponent{{1.f, 2.f}, 0.5f});

    Entity child = scene.create_entity();
    child.add_component<TagComponent>(TagComponent{"Child"});
    child.add_component<ChildComponent>(ChildComponent{parent});

    FileHandle file = DiskFileProvider().open("scene_serializer_round_trip.json");
    save_scene(scene, engine, assets, file);

    Scene loaded;
    load_scene(loaded, engine, assets, file);
    file.remove();

    ASSERT_TRUE(loaded.get_systems().has<GravitySystem>());
    EXPECT_FLOAT_EQ(loaded.get_systems().get<GravitySystem>().strength, 12.5f);

    Entity loadedParent = find_by_tag(loaded, "Parent");
    Entity loadedChild = find_by_tag(loaded, "Child");
    ASSERT_TRUE(loadedParent.is_valid());
    ASSERT_TRUE(loadedChild.is_valid());

    ASSERT_TRUE(loadedParent.has_component<TransformComponent>());
    EXPECT_FLOAT_EQ(loadedParent.get_component<TransformComponent>().position.x, 1.f);
    EXPECT_FLOAT_EQ(loadedParent.get_component<TransformComponent>().position.y, 2.f);
    EXPECT_FLOAT_EQ(loadedParent.get_component<TransformComponent>().rotation, 0.5f);

    ASSERT_TRUE(loadedChild.has_component<ChildComponent>());
    EXPECT_EQ(loadedChild.get_component<ChildComponent>().parent, loadedParent);

    ASSERT_TRUE(loadedParent.has_component<ParentComponent>());
    ASSERT_EQ(loadedParent.get_component<ParentComponent>().children.size(), 1u);
    EXPECT_EQ(loadedParent.get_component<ParentComponent>().children[0], loadedChild);
}

TEST(SceneSerializer, UnregisteredSystemsAndComponentsAreSkippedRatherThanThrowing)
{
    Engine engine; // TransformComponent registered, TagComponent is not exercised here
    AssetManager assets;

    Scene scene;
    scene.get_systems().add<GravitySystem>(); // attached directly, never registered in engine.systems()

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>(TransformComponent{{7.f, 8.f}, 0.f});

    FileHandle file = DiskFileProvider().open("scene_serializer_unregistered.json");
    ASSERT_NO_THROW(save_scene(scene, engine, assets, file));

    Scene loaded;
    ASSERT_NO_THROW(load_scene(loaded, engine, assets, file));
    file.remove();

    EXPECT_FALSE(loaded.get_systems().has<GravitySystem>()); // never in the file, GravitySystem has no catalog entry

    Entity loadedEntity;
    for(entt::entity raw : loaded.get_registry().storage<entt::entity>()){
        Entity e(&loaded, raw);
        if(e.has_component<TransformComponent>())
            loadedEntity = e;
    }

    ASSERT_TRUE(loadedEntity.is_valid());
    EXPECT_FLOAT_EQ(loadedEntity.get_component<TransformComponent>().position.x, 7.f);
}

TEST(SceneSerializer, PhysicsBodyAndColliderMaterializeAutomaticallyOnLoad)
{
    Engine engine;
    AssetManager assets;

    World world({0.f, -9.8f});
    Scene scene;

    // PhysicsSystem's Scene& comes from whichever scene add() is called for (see
    // system_catalog.hpp/SystemFactory), but World& is still captured here. This means
    // load_scene()
    engine.systems().register_system<PhysicsSystem>([&world](Scene& scene){ return std::make_unique<PhysicsSystem>(scene, world); });
    engine.systems().by_type<PhysicsSystem>()->add(scene);

    Entity entity = scene.create_entity();
    entity.add_component<TransformComponent>(TransformComponent{{3.f, 4.f}, 0.f});
    entity.add_component<RigidBodyComponent>(RigidBodyComponent{.type = BodyType::DYNAMIC});

    CircleShape circle;
    entity.add_component<ColliderComponent>(ColliderComponent(circle));

    ASSERT_TRUE(entity.has_component<NativeBodyComponent>());
    ASSERT_EQ(world.get_body_count(), 1u);

    FileHandle file = DiskFileProvider().open("scene_serializer_physics.json");
    save_scene(scene, engine, assets, file);

    // Simulate starting from nothing, the way a real load would: no entity, no native body.
    entity.destroy();
    ASSERT_EQ(world.get_body_count(), 0u);

    load_scene(scene, engine, assets, file);
    file.remove();

    // No explicit physics re-creation code above. PhysicsSystem's on_construct hooks (already
    // attached by load pass 2, before load pass 3 adds RigidBodyComponent/ColliderComponent back)
    // did this automatically, exactly as they would at normal runtime.
    ASSERT_EQ(world.get_body_count(), 1u);

    Entity loaded;
    for(entt::entity raw : scene.get_registry().storage<entt::entity>()){
        Entity e(&scene, raw);
        if(e.has_component<RigidBodyComponent>())
            loaded = e;
    }

    ASSERT_TRUE(loaded.is_valid());
    ASSERT_TRUE(loaded.has_component<NativeBodyComponent>());
    EXPECT_FLOAT_EQ(loaded.get_component<NativeBodyComponent>().bodyPtr->get_position().x, 3.f);
    EXPECT_FLOAT_EQ(loaded.get_component<NativeBodyComponent>().bodyPtr->get_position().y, 4.f);

    ASSERT_TRUE(loaded.has_component<ColliderComponent>());
    EXPECT_TRUE(loaded.get_component<ColliderComponent>().collider.is_attached());
}
