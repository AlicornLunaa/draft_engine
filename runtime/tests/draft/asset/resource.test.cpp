#include <gtest/gtest.h>
#include "draft/asset/resource.hpp"

using namespace Draft;

TEST(Resource, DefaultIsInvalid)
{
    Resource<int> r;
    ASSERT_FALSE(r.is_valid());
    ASSERT_FALSE(static_cast<bool>(r));
    ASSERT_EQ(r.get(), nullptr);
}

TEST(Resource, WrapsASlotsValue)
{
    auto slot = std::make_shared<AssetSlot<int>>(std::make_shared<int>(42));
    Resource<int> r(slot);

    ASSERT_TRUE(r.is_valid());
    ASSERT_EQ(*r, 42);
    ASSERT_EQ(*r.get(), 42);
}

TEST(Resource, EmptySlotIsInvalid)
{
    auto slot = std::make_shared<AssetSlot<int>>();
    Resource<int> r(slot);

    ASSERT_FALSE(r.is_valid());
}

TEST(Resource, SwappingTheSlotUpdatesEveryHandle)
{
    auto slot = std::make_shared<AssetSlot<int>>(std::make_shared<int>(1));
    Resource<int> a(slot);
    Resource<int> b(slot); // shares the same underlying slot

    ASSERT_EQ(*a, 1);
    ASSERT_EQ(*b, 1);

    slot->set(std::make_shared<int>(2));

    // Neither handle was re-requested, but both observe the new value.
    ASSERT_EQ(*a, 2);
    ASSERT_EQ(*b, 2);
}

TEST(Resource, UnloadingSetsSlotToNull)
{
    auto slot = std::make_shared<AssetSlot<int>>(std::make_shared<int>(5));
    Resource<int> r(slot);
    ASSERT_TRUE(r.is_valid());

    slot->set(nullptr);
    ASSERT_FALSE(r.is_valid());
}

TEST(Resource, GetSharedPinsTheCurrentValueAcrossASwap)
{
    auto slot = std::make_shared<AssetSlot<int>>(std::make_shared<int>(10));
    Resource<int> r(slot);

    std::shared_ptr<int> pinned = r.get_shared();
    ASSERT_EQ(*pinned, 10);

    slot->set(std::make_shared<int>(20));

    // The resource itself now reflects the swap...
    ASSERT_EQ(*r, 20);
    // ...but the previously-pinned shared_ptr still owns the old value.
    ASSERT_EQ(*pinned, 10);
}

TEST(Resource, CopyingASharesTheSlot)
{
    auto slot = std::make_shared<AssetSlot<int>>(std::make_shared<int>(7));
    Resource<int> a(slot);
    Resource<int> b = a;

    slot->set(std::make_shared<int>(8));
    ASSERT_EQ(*a, 8);
    ASSERT_EQ(*b, 8);
}
