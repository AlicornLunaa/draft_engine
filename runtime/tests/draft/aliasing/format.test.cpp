#include <gtest/gtest.h>
#include "draft/aliasing/format.hpp"

TEST(Format, ChannelsToColorFormat)
{
    ASSERT_EQ(Draft::channels_to_color_format(1), Draft::ColorFormat::GREYSCALE);
    ASSERT_EQ(Draft::channels_to_color_format(2), Draft::ColorFormat::RG);
    ASSERT_EQ(Draft::channels_to_color_format(3), Draft::ColorFormat::RGB);
    ASSERT_EQ(Draft::channels_to_color_format(4), Draft::ColorFormat::RGBA);
}

TEST(Format, ColorFormatToBytes)
{
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::GREYSCALE), 1u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::RG), 2u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::RGB), 3u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::RGBA), 4u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::DEPTH_COMPONENT), 1u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::DEPTH_COMPONENT16), 2u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::DEPTH_COMPONENT24), 3u);
    ASSERT_EQ(Draft::color_format_to_bytes(Draft::ColorFormat::DEPTH_STENCIL_COMPONENT), 1u);
}
