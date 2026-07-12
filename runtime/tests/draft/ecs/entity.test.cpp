#include <gtest/gtest.h>
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"

using namespace Draft;

namespace {
    struct Position {
        float x = 0.f;
        float y = 0.f;
    };

    struct Velocity {
        float dx = 0.f;
        float dy = 0.f;
    };
}

TEST(Entity, DefaultConstructedIsInvalid)
{
    Entity e;
    ASSERT_FALSE(e.is_valid());
    ASSERT_FALSE(static_cast<bool>(e));
}

TEST(Entity, NullEntityIsEquivalentToDefault)
{
    ASSERT_FALSE(NULL_ENTITY.is_valid());
    ASSERT_EQ(Entity(), NULL_ENTITY);
}

TEST(Entity, CreatedEntityIsValid)
{
    Scene scene;
    Entity e = scene.create_entity();
    ASSERT_TRUE(e.is_valid());
    ASSERT_TRUE(static_cast<bool>(e));
}

TEST(Entity, AddAndGetComponent)
{
    Scene scene;
    Entity e = scene.create_entity();

    e.add_component<Position>(Position{1.f, 2.f});
    ASSERT_TRUE(e.has_component<Position>());
    ASSERT_FLOAT_EQ(e.get_component<Position>().x, 1.f);
    ASSERT_FLOAT_EQ(e.get_component<Position>().y, 2.f);
}

TEST(Entity, HasComponentIsFalseWhenNotAdded)
{
    Scene scene;
    Entity e = scene.create_entity();
    ASSERT_FALSE(e.has_component<Position>());
}

TEST(Entity, HasComponentSupportsMultipleTypes)
{
    Scene scene;
    Entity e = scene.create_entity();
    e.add_component<Position>();
    e.add_component<Velocity>();

    ASSERT_TRUE((e.has_component<Position, Velocity>()));
}

TEST(Entity, RemoveComponent)
{
    Scene scene;
    Entity e = scene.create_entity();
    e.add_component<Position>();
    ASSERT_TRUE(e.has_component<Position>());

    e.remove_component<Position>();
    ASSERT_FALSE(e.has_component<Position>());
}

TEST(Entity, TryGetComponentReturnsNullptrWhenMissing)
{
    Scene scene;
    Entity e = scene.create_entity();
    ASSERT_EQ(e.try_get_component<Position>(), nullptr);

    e.add_component<Position>(Position{3.f, 4.f});
    Position* p = e.try_get_component<Position>();
    ASSERT_NE(p, nullptr);
    ASSERT_FLOAT_EQ(p->x, 3.f);
}

TEST(Entity, TryGetComponentIsSafeOnAnInvalidHandle)
{
    Entity e; // default-constructed, invalid
    ASSERT_EQ(e.try_get_component<Position>(), nullptr);
}

TEST(Entity, ModifyComponentTriggersInPlaceUpdate)
{
    Scene scene;
    Entity e = scene.create_entity();
    e.add_component<Position>(Position{0.f, 0.f});

    e.modify_component<Position>([](Position& p){ p.x = 42.f; });
    ASSERT_FLOAT_EQ(e.get_component<Position>().x, 42.f);
}

TEST(Entity, DestroyInvalidatesTheHandle)
{
    Scene scene;
    Entity e = scene.create_entity();
    ASSERT_TRUE(e.is_valid());

    ASSERT_TRUE(e.destroy());
    ASSERT_FALSE(e.is_valid());
}

TEST(Entity, DestroyingAnAlreadyInvalidHandleReturnsFalse)
{
    Entity e;
    ASSERT_FALSE(e.destroy());
}

TEST(Entity, EqualityHoldsForTheSameEntity)
{
    Scene scene;
    Entity e = scene.create_entity();
    Entity copy = e;
    ASSERT_EQ(e, copy);
}

TEST(Entity, EntitiesFromDifferentScenesWithTheSameRawIdAreNotEqual)
{
    // Regression test
    Scene sceneA;
    Scene sceneB;

    Entity a = sceneA.create_entity();
    Entity b = sceneB.create_entity();

    ASSERT_EQ(static_cast<entt::entity>(a), static_cast<entt::entity>(b));
    ASSERT_NE(a, b);
}

TEST(Entity, GetScene)
{
    Scene scene;
    Entity e = scene.create_entity();
    ASSERT_EQ(e.get_scene(), &scene);
}
