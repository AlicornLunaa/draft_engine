#include <gtest/gtest.h>
#include "draft/rendering/image.hpp"
#include "draft/util/files/host_file_system.hpp"

using namespace Draft;

TEST(Image, SolidColorConstructorFillsEveryPixel)
{
    Image img({2, 2}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);

    for(unsigned int y = 0; y < 2; y++){
        for(unsigned int x = 0; x < 2; x++){
            Vector4f pixel = img.get_pixel({x, y});
            ASSERT_FLOAT_EQ(pixel.r, 1.f);
            ASSERT_FLOAT_EQ(pixel.g, 0.f);
            ASSERT_FLOAT_EQ(pixel.b, 0.f);
            ASSERT_FLOAT_EQ(pixel.a, 1.f);
        }
    }
}

TEST(Image, RawDataConstructorCopiesBytesVerbatim)
{
    std::byte raw[] = {
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
    };

    Image img({2, 1}, ColorFormat::RGBA, raw);

    Vector4f left = img.get_pixel({0, 0});
    Vector4f right = img.get_pixel({1, 0});
    ASSERT_FLOAT_EQ(left.r, 1.f);
    ASSERT_FLOAT_EQ(left.g, 0.f);
    ASSERT_FLOAT_EQ(right.r, 0.f);
    ASSERT_FLOAT_EQ(right.g, 1.f);
}

TEST(Image, SetPixelThenGetPixelRoundTripsRGBA)
{
    Image img({2, 2}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);
    img.set_pixel({1, 0}, {0.2f, 0.4f, 0.6f, 0.8f});

    Vector4f pixel = img.get_pixel({1, 0});
    ASSERT_NEAR(pixel.r, 0.2f, 0.01f);
    ASSERT_NEAR(pixel.g, 0.4f, 0.01f);
    ASSERT_NEAR(pixel.b, 0.6f, 0.01f);
    ASSERT_NEAR(pixel.a, 0.8f, 0.01f);
}

TEST(Image, SetPixelThenGetPixelRoundTripsGreyscale)
{
    Image img({1, 1}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::GREYSCALE);
    img.set_pixel({0, 0}, {0.5f, 0.5f, 0.5f, 1.f});

    Vector4f pixel = img.get_pixel({0, 0});
    ASSERT_NEAR(pixel.r, 0.5f, 0.01f);
    ASSERT_NEAR(pixel.g, 0.5f, 0.01f);
    ASSERT_NEAR(pixel.b, 0.5f, 0.01f);
}

TEST(Image, FlipHorizontallySwapsLeftAndRight)
{
    std::byte raw[] = {
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
    };

    Image img({2, 1}, ColorFormat::RGBA, raw);
    img.flip_horizontally();

    ASSERT_FLOAT_EQ(img.get_pixel({0, 0}).g, 1.f);
    ASSERT_FLOAT_EQ(img.get_pixel({1, 0}).r, 1.f);
}

TEST(Image, FlipVerticallySwapsTopAndBottom)
{
    std::byte raw[] = {
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{255}, std::byte{0}, std::byte{255},
    };

    Image img({1, 2}, ColorFormat::RGBA, raw);
    img.flip_vertically();

    ASSERT_FLOAT_EQ(img.get_pixel({0, 0}).g, 1.f);
    ASSERT_FLOAT_EQ(img.get_pixel({0, 1}).r, 1.f);
}

TEST(Image, MaskZeroesUnmatchedPixelsAndUsesProvidedAlpha)
{
    std::byte raw[] = {
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255}, // matches mask color
        std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255}, // doesn't match
    };

    Image img({2, 1}, ColorFormat::RGBA, raw);
    img.mask({1.f, 0.f, 0.f, 1.f}, 0.1f, std::byte{0x7F});

    // Matched pixel is untouched
    ASSERT_FLOAT_EQ(img.get_pixel({0, 0}).r, 1.f);

    // Unmatched pixel's alpha is set to exactly the provided `alpha` value, not hardcoded 0
    // regression test for a bug found during the port where `alpha` was accepted but never used.
    float expectedAlpha = 0x7F / 255.f;
    ASSERT_NEAR(img.get_pixel({1, 0}).a, expectedAlpha, 0.01f);
}

TEST(Image, ConvertChangesFormatAndPreservesColor)
{
    Image img({1, 1}, {0.6f, 0.6f, 0.6f, 1.f}, ColorFormat::RGBA);
    img.convert(ColorFormat::GREYSCALE);

    ASSERT_EQ(img.get_format(), ColorFormat::GREYSCALE);
    ASSERT_NEAR(img.get_pixel({0, 0}).r, 0.6f, 0.02f);
}

TEST(Image, CopyWithApplyAlphaScalesEveryPixelNotJustTheFirst)
{
    // Regression test for a bug found while porting Image::copy()
    std::byte raw[] = {
        std::byte{255}, std::byte{255}, std::byte{255}, std::byte{255}, // (0,0) alpha=255
        std::byte{255}, std::byte{255}, std::byte{255}, std::byte{128}, // (1,0) alpha=128
        std::byte{255}, std::byte{255}, std::byte{255}, std::byte{64},  // (0,1) alpha=64
        std::byte{255}, std::byte{255}, std::byte{255}, std::byte{32},  // (1,1) alpha=32
    };
    Image src({2, 2}, ColorFormat::RGBA, raw);

    Image dest({2, 2}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);
    dest.copy(src, {0, 0}, {0, 0, 0, 0}, true);

    // Every source pixel is opaque white; applying its own alpha as a scalar on the white channel
    // should reproduce alpha/255 in each color channel at that same pixel.
    ASSERT_NEAR(dest.get_pixel({0, 0}).r, 255 / 255.f, 0.02f);
    ASSERT_NEAR(dest.get_pixel({1, 0}).r, 128 / 255.f, 0.02f);
    ASSERT_NEAR(dest.get_pixel({0, 1}).r, 64 / 255.f, 0.02f);
    ASSERT_NEAR(dest.get_pixel({1, 1}).r, 32 / 255.f, 0.02f);
}

TEST(Image, CopyWithoutApplyAlphaCopiesBytesDirectly)
{
    Image src({1, 1}, {0.25f, 0.5f, 0.75f, 1.f}, ColorFormat::RGBA);
    Image dest({1, 1}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);
    dest.copy(src, {0, 0});

    Vector4f pixel = dest.get_pixel({0, 0});
    ASSERT_NEAR(pixel.r, 0.25f, 0.01f);
    ASSERT_NEAR(pixel.g, 0.5f, 0.01f);
    ASSERT_NEAR(pixel.b, 0.75f, 0.01f);
}

TEST(Image, CopyOutOfBoundsPixelsAreSkippedNotCrashed)
{
    Image src({2, 2}, {1.f, 1.f, 1.f, 1.f}, ColorFormat::RGBA);
    Image dest({1, 1}, {0.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);

    ASSERT_NO_THROW(dest.copy(src, {0, 0}));
}

TEST(Image, CopyAndMoveConstructorsAreIndependentCopies)
{
    Image original({1, 1}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA);
    Image copy(original);
    copy.set_pixel({0, 0}, {0.f, 1.f, 0.f, 1.f});

    ASSERT_FLOAT_EQ(original.get_pixel({0, 0}).r, 1.f);
    ASSERT_FLOAT_EQ(copy.get_pixel({0, 0}).g, 1.f);

    Image moved(std::move(copy));
    ASSERT_FLOAT_EQ(moved.get_pixel({0, 0}).g, 1.f);
}

TEST(Image, SaveThenLoadRoundTripsThroughARealFile)
{
    HostFileSystem fs;

    Image original({2, 2}, {0.f, 0.f, 1.f, 1.f}, ColorFormat::RGBA);
    original.save(fs.open("test_image_roundtrip.png"));

    ASSERT_TRUE(fs.exists("test_image_roundtrip.png"));

    Image loaded(fs.open("test_image_roundtrip.png"));
    ASSERT_EQ(loaded.get_size().x, 2u);
    ASSERT_EQ(loaded.get_size().y, 2u);
    ASSERT_NEAR(loaded.get_pixel({0, 0}).b, 1.f, 0.02f);

    fs.remove("test_image_roundtrip.png");
}
