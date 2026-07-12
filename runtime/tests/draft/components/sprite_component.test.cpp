#include <gtest/gtest.h>
#include "draft/components/sprite_component.hpp"

using namespace Draft;

TEST(SpriteComponent, DefaultConstructionLeavesAnEmptyTextureAndTheDocumentedDefaults)
{
    SpriteComponent sprite;

    EXPECT_FALSE(sprite.texture.texture.is_valid());
    EXPECT_FLOAT_EQ(sprite.size.x, 64.f);
    EXPECT_FLOAT_EQ(sprite.size.y, 64.f);
    EXPECT_FLOAT_EQ(sprite.origin.x, 0.f);
    EXPECT_FLOAT_EQ(sprite.origin.y, 0.f);
    EXPECT_FLOAT_EQ(sprite.zIndex, 0.f);
}

TEST(SpriteComponent, ResourceConstructorWrapsTheTextureWithEmptyBounds)
{
    Resource<Texture> texture; // empty is a valid Resource<T>, no GL needed for this test

    SpriteComponent sprite(texture, {32, 16}, {1, 2});

    EXPECT_FLOAT_EQ(sprite.size.x, 32.f);
    EXPECT_FLOAT_EQ(sprite.size.y, 16.f);
    EXPECT_FLOAT_EQ(sprite.origin.x, 1.f);
    EXPECT_FLOAT_EQ(sprite.origin.y, 2.f);
    EXPECT_FLOAT_EQ(sprite.texture.bounds.width, 0.f);
    EXPECT_FLOAT_EQ(sprite.texture.bounds.height, 0.f);
}

TEST(SpriteComponent, TextureRegionConstructorPreservesTheGivenBounds)
{
    TextureRegion region;
    region.bounds = {4.f, 5.f, 6.f, 7.f};

    SpriteComponent sprite(region, {8, 9});

    EXPECT_FLOAT_EQ(sprite.texture.bounds.x, 4.f);
    EXPECT_FLOAT_EQ(sprite.texture.bounds.y, 5.f);
    EXPECT_FLOAT_EQ(sprite.texture.bounds.width, 6.f);
    EXPECT_FLOAT_EQ(sprite.texture.bounds.height, 7.f);
    EXPECT_FLOAT_EQ(sprite.origin.x, 0.f); // Default origin
    EXPECT_FLOAT_EQ(sprite.origin.y, 0.f);
}
