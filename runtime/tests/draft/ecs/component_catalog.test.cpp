#include <gtest/gtest.h>
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/scene.hpp"

#include <stdexcept>
#include <typeindex>
#include <vector>

using namespace Draft;

namespace {
    struct Position {
        DRAFT_REFLECTED(float, x) = 0.f;
        DRAFT_REFLECTED(float, y) = 0.f;

        DRAFT_REFLECTABLE(Position, x, y)
    };

    struct Velocity {
        DRAFT_REFLECTED(float, dx) = 0.f;
        DRAFT_REFLECTED(float, dy) = 0.f;

        DRAFT_REFLECTABLE(Velocity, dx, dy)
    };

    // Hand-written (not DRAFT_REFLECTABLE) Reflectable types that deliberately share a
    // reflect_name(), to exercise the name-collision guard even though two macro-declared types
    // can't normally collide (their reflected name always matches their own C++ type name).
    struct SharedNameA {
        static constexpr auto reflect(){ return std::make_tuple(); }
        static constexpr std::string_view reflect_name(){ return "Shared"; }
    };

    struct SharedNameB {
        static constexpr auto reflect(){ return std::make_tuple(); }
        static constexpr std::string_view reflect_name(){ return "Shared"; }
    };
}

TEST(ComponentCatalog, ByTypeReturnsNullptrWhenNotRegistered)
{
    ComponentCatalog catalog;
    ASSERT_EQ(catalog.by_type<Position>(), nullptr);
}

TEST(ComponentCatalog, ByNameReturnsNullptrWhenNotRegistered)
{
    ComponentCatalog catalog;
    ASSERT_EQ(catalog.by_name("Position"), nullptr);
}

TEST(ComponentCatalog, RegisterThenByTypeReturnsTheEntry)
{
    ComponentCatalog catalog;
    catalog.register_component<Position>();

    ComponentTypeInterface* entry = catalog.by_type<Position>();
    ASSERT_NE(entry, nullptr);
    ASSERT_EQ(entry->name(), "Position");
    ASSERT_EQ(entry->type(), std::type_index(typeid(Position)));
}

TEST(ComponentCatalog, RegisterThenByNameReturnsTheSameEntry)
{
    ComponentCatalog catalog;
    catalog.register_component<Position>();

    ASSERT_EQ(catalog.by_name("Position"), catalog.by_type<Position>());
}

TEST(ComponentCatalog, AllReturnsRegisteredTypesInRegistrationOrder)
{
    ComponentCatalog catalog;
    catalog.register_component<Velocity>();
    catalog.register_component<Position>();

    const auto& all = catalog.all();
    ASSERT_EQ(all.size(), 2u);
    ASSERT_EQ(all[0]->name(), "Velocity");
    ASSERT_EQ(all[1]->name(), "Position");
}

TEST(ComponentCatalog, ReRegisteringTheSameTypeReplacesRatherThanDuplicating)
{
    ComponentCatalog catalog;
    catalog.register_component<Position>();
    catalog.register_component<Position>(); // same type, registered again

    ASSERT_EQ(catalog.all().size(), 1u);
    ASSERT_NE(catalog.by_name("Position"), nullptr);
}

TEST(ComponentCatalog, ReRegisteringKeepsItsOriginalPositionInOrder)
{
    ComponentCatalog catalog;
    catalog.register_component<Velocity>();
    catalog.register_component<Position>();
    catalog.register_component<Velocity>(); // re-registered, should not move to the back

    const auto& all = catalog.all();
    ASSERT_EQ(all.size(), 2u);
    ASSERT_EQ(all[0]->name(), "Velocity");
    ASSERT_EQ(all[1]->name(), "Position");
}

TEST(ComponentCatalog, RegisteringADifferentTypeUnderAnAlreadyUsedNameThrows)
{
    ComponentCatalog catalog;
    catalog.register_component<SharedNameA>();
    ASSERT_THROW(catalog.register_component<SharedNameB>(), std::logic_error);
}

TEST(ComponentCatalog, AddDefaultAddsADefaultConstructedComponent)
{
    Scene scene;
    Entity e = scene.create_entity();

    ComponentCatalog catalog;
    catalog.register_component<Position>();

    catalog.by_type<Position>()->add_default(e);
    ASSERT_TRUE(e.has_component<Position>());
    ASSERT_FLOAT_EQ(e.get_component<Position>().x, 0.f);
}

TEST(ComponentCatalog, RemoveRemovesTheComponentFromTheEntity)
{
    Scene scene;
    Entity e = scene.create_entity();
    e.add_component<Position>(Position{1.f, 2.f});

    ComponentCatalog catalog;
    catalog.register_component<Position>();

    catalog.by_type<Position>()->remove(e);
    ASSERT_FALSE(e.has_component<Position>());
}

TEST(ComponentCatalog, HasReflectsWhetherTheEntityHasTheComponent)
{
    Scene scene;
    Entity e = scene.create_entity();

    ComponentCatalog catalog;
    catalog.register_component<Position>();

    ComponentTypeInterface* entry = catalog.by_type<Position>();
    ASSERT_FALSE(entry->has(e));

    e.add_component<Position>();
    ASSERT_TRUE(entry->has(e));
}

TEST(ComponentCatalog, CloneCopiesTheComponentValueToAnotherEntity)
{
    Scene scene;
    Entity src = scene.create_entity();
    Entity dst = scene.create_entity();
    src.add_component<Position>(Position{3.f, 4.f});

    ComponentCatalog catalog;
    catalog.register_component<Position>();

    catalog.by_type<Position>()->clone(src, dst);
    ASSERT_TRUE(dst.has_component<Position>());
    ASSERT_FLOAT_EQ(dst.get_component<Position>().x, 3.f);
    ASSERT_FLOAT_EQ(dst.get_component<Position>().y, 4.f);
}

TEST(ComponentCatalog, CloneIsANoOpWhenTheSourceLacksTheComponent)
{
    Scene scene;
    Entity src = scene.create_entity();
    Entity dst = scene.create_entity();

    ComponentCatalog catalog;
    catalog.register_component<Position>();

    catalog.by_type<Position>()->clone(src, dst);
    ASSERT_FALSE(dst.has_component<Position>());
}
