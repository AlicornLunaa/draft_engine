#include <gtest/gtest.h>
#include "draft/ecs/system_catalog.hpp"

#include <memory>
#include <stdexcept>
#include <typeindex>

using namespace Draft;

namespace {
    // A system holding its own reflected, savable data alongside its per-tick behavior, the
    // same shape as the real AudioSystem::dopplerSensitivity.
    struct GravitySystem : AbstractSystem {
        DRAFT_REFLECTED(float, strength) = 9.8f;

        void update(Time) override {}

        DRAFT_REFLECTABLE(GravitySystem, strength)
    };

    struct OtherSystem : AbstractSystem {
        DRAFT_REFLECTABLE(OtherSystem)
    };

    SystemFactory gravity_factory(){
        return []{ return std::make_unique<GravitySystem>(); };
    }

    SystemFactory other_factory(){
        return []{ return std::make_unique<OtherSystem>(); };
    }

    // Hand-written (not DRAFT_REFLECTABLE) Reflectable types that deliberately share a
    // reflect_name(), to exercise the name-collision guard even though two macro-declared types
    // can't normally collide (their reflected name always matches their own C++ type name).
    struct SharedNameA : AbstractSystem {
        int unused = 0;
        static constexpr auto reflect(){ return std::make_tuple(); }
        static constexpr std::string_view reflect_name(){ return "Shared"; }
    };

    struct SharedNameB : AbstractSystem {
        int unused = 0;
        static constexpr auto reflect(){ return std::make_tuple(); }
        static constexpr std::string_view reflect_name(){ return "Shared"; }
    };
}

TEST(SystemCatalog, ByTypeReturnsNullptrWhenNotRegistered)
{
    SystemCatalog catalog;
    ASSERT_EQ(catalog.by_type<GravitySystem>(), nullptr);
}

TEST(SystemCatalog, ByNameReturnsNullptrWhenNotRegistered)
{
    SystemCatalog catalog;
    ASSERT_EQ(catalog.by_name("GravitySystem"), nullptr);
}

TEST(SystemCatalog, RegisterThenByTypeReturnsTheEntry)
{
    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());

    SystemTypeInterface* entry = catalog.by_type<GravitySystem>();
    ASSERT_NE(entry, nullptr);
    ASSERT_EQ(entry->name(), "GravitySystem");
    ASSERT_EQ(entry->type(), std::type_index(typeid(GravitySystem)));
}

TEST(SystemCatalog, RegisterThenByNameReturnsTheSameEntry)
{
    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());

    ASSERT_EQ(catalog.by_name("GravitySystem"), catalog.by_type<GravitySystem>());
}

TEST(SystemCatalog, AllReturnsRegisteredTypesInRegistrationOrder)
{
    SystemCatalog catalog;
    catalog.register_system<OtherSystem>(other_factory());
    catalog.register_system<GravitySystem>(gravity_factory());

    const auto& all = catalog.all();
    ASSERT_EQ(all.size(), 2u);
    ASSERT_EQ(all[0]->name(), "OtherSystem");
    ASSERT_EQ(all[1]->name(), "GravitySystem");
}

TEST(SystemCatalog, ReRegisteringTheSameTypeReplacesRatherThanDuplicating)
{
    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());
    catalog.register_system<GravitySystem>(gravity_factory()); // same type, registered again

    ASSERT_EQ(catalog.all().size(), 1u);
    ASSERT_NE(catalog.by_name("GravitySystem"), nullptr);
}

TEST(SystemCatalog, ReRegisteringKeepsItsOriginalPositionInOrder)
{
    SystemCatalog catalog;
    catalog.register_system<OtherSystem>(other_factory());
    catalog.register_system<GravitySystem>(gravity_factory());
    catalog.register_system<OtherSystem>(other_factory()); // re-registered, should not move to the back

    const auto& all = catalog.all();
    ASSERT_EQ(all.size(), 2u);
    ASSERT_EQ(all[0]->name(), "OtherSystem");
    ASSERT_EQ(all[1]->name(), "GravitySystem");
}

TEST(SystemCatalog, RegisteringADifferentTypeUnderAnAlreadyUsedNameThrows)
{
    SystemCatalog catalog;
    catalog.register_system<SharedNameA>([]{ return std::make_unique<SharedNameA>(); });
    ASSERT_THROW(catalog.register_system<SharedNameB>([]{ return std::make_unique<SharedNameB>(); }), std::logic_error);
}

TEST(SystemCatalog, AddConstructsViaTheFactoryAndAttachesIt)
{
    SystemRegistry registry;

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());

    catalog.by_type<GravitySystem>()->add(registry);
    ASSERT_TRUE(registry.has<GravitySystem>());
    ASSERT_FLOAT_EQ(registry.get<GravitySystem>().strength, 9.8f);
}

TEST(SystemCatalog, RemoveRemovesTheSystemFromTheRegistry)
{
    SystemRegistry registry;
    registry.add<GravitySystem>();

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());

    catalog.by_type<GravitySystem>()->remove(registry);
    ASSERT_FALSE(registry.has<GravitySystem>());
}

TEST(SystemCatalog, HasReflectsWhetherTheRegistryHasTheSystem)
{
    SystemRegistry registry;

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());

    SystemTypeInterface* entry = catalog.by_type<GravitySystem>();
    ASSERT_FALSE(entry->has(registry));

    registry.add<GravitySystem>();
    ASSERT_TRUE(entry->has(registry));
}

TEST(SystemCatalog, GenericJsonSerializeRoundTripsThroughSystemTypeInterface)
{
    SystemRegistry src;
    src.add<GravitySystem>().strength = 3.5f;

    SystemRegistry dst;
    dst.add<GravitySystem>(); // must already be attached, unlike a component's deserialize()

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());
    SystemTypeInterface* entry = catalog.by_type<GravitySystem>();

    JSON json;
    entry->serialize(src, json);
    entry->deserialize(dst, json);

    ASSERT_FLOAT_EQ(dst.get<GravitySystem>().strength, 3.5f);
}

TEST(SystemCatalog, GenericBinarySerializeRoundTripsThroughSystemTypeInterface)
{
    SystemRegistry src;
    src.add<GravitySystem>().strength = 7.25f;

    SystemRegistry dst;
    dst.add<GravitySystem>();

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());
    SystemTypeInterface* entry = catalog.by_type<GravitySystem>();

    Binary::ByteArray buffer;
    entry->serialize(src, buffer);

    Binary::ByteView view(buffer);
    entry->deserialize(dst, view);

    ASSERT_FLOAT_EQ(dst.get<GravitySystem>().strength, 7.25f);
}

TEST(SystemCatalog, DeserializeThrowsWhenTheSystemIsntAlreadyAttached)
{
    SystemRegistry src;
    src.add<GravitySystem>();

    SystemRegistry dst; // GravitySystem never added() here

    SystemCatalog catalog;
    catalog.register_system<GravitySystem>(gravity_factory());
    SystemTypeInterface* entry = catalog.by_type<GravitySystem>();

    JSON json;
    entry->serialize(src, json);
    ASSERT_THROW(entry->deserialize(dst, json), std::logic_error);
}
