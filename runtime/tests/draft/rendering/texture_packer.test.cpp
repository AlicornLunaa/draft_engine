#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/rendering/texture_packer.hpp"
#include "draft/rendering/render_window.hpp"

#include "GLFW/glfw3.h"

using namespace Draft;

TEST(TexturePacker, PackPlacesNonOverlappingRegionsForFittingImages)
{
    TexturePacker packer;

    std::vector<std::pair<std::string, Image>> data;
    data.emplace_back("red", Image({20, 20}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA));
    data.emplace_back("green", Image({20, 20}, {0.f, 1.f, 0.f, 1.f}, ColorFormat::RGBA));

    packer.pack(data);

    // Both regions should have been placed and not overlap.
    TextureRegion red = packer.get_region("red");
    TextureRegion green = packer.get_region("green");
    ASSERT_NE(red.bounds.width, 0.f);
    ASSERT_NE(green.bounds.width, 0.f);
    EXPECT_FALSE(
        red.bounds.x < green.bounds.x + green.bounds.width &&
        red.bounds.x + red.bounds.width > green.bounds.x &&
        red.bounds.y < green.bounds.y + green.bounds.height &&
        red.bounds.y + red.bounds.height > green.bounds.y
    );
}

TEST(TexturePacker, PackSurvivesAMidScanCanvasResizeWithoutCorruption)
{
    TexturePacker packer;

    std::vector<std::pair<std::string, Image>> data;
    data.emplace_back("a", Image({120, 50}, {1.f, 0.f, 0.f, 1.f}, ColorFormat::RGBA));
    data.emplace_back("b", Image({120, 50}, {0.f, 1.f, 0.f, 1.f}, ColorFormat::RGBA));
    data.emplace_back("c", Image({120, 50}, {0.f, 0.f, 1.f, 1.f}, ColorFormat::RGBA));

    packer.pack(data);

    // Confirm the resize actually happened, otherwise this test isn't exercising the bug at all.
    ASSERT_GT(packer.get_image().get_size().x, 128u);

    // All three images should have been placed, each still showing its own solid color at the
    // center of its reported region
    for(auto& [name, expectedColor] : std::vector<std::pair<std::string, Vector4f>>{
        {"a", {1.f, 0.f, 0.f, 1.f}},
        {"b", {0.f, 1.f, 0.f, 1.f}},
        {"c", {0.f, 0.f, 1.f, 1.f}},
    }){
        TextureRegion region = packer.get_region(name);
        ASSERT_GT(region.bounds.width, 0.f) << "region for " << name << " was never placed";

        Vector2u center{
            static_cast<unsigned int>(region.bounds.x + region.bounds.width / 2),
            static_cast<unsigned int>(region.bounds.y + region.bounds.height / 2)
        };
        Vector4f pixel = packer.get_image().get_pixel(center);
        EXPECT_NEAR(pixel.r, expectedColor.r, 0.01f) << name;
        EXPECT_NEAR(pixel.g, expectedColor.g, 0.01f) << name;
        EXPECT_NEAR(pixel.b, expectedColor.b, 0.01f) << name;
    }
}

class TexturePackerCreateTest : public ::testing::Test {
protected:
    static RenderWindow* window;

    static void SetUpTestSuite(){
        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "texture_packer_test");
    }

    static void TearDownTestSuite(){
        delete window;
        window = nullptr;
    }
};

RenderWindow* TexturePackerCreateTest::window = nullptr;

TEST_F(TexturePackerCreateTest, CreateUploadsARealTextureAndRegionsStayValid)
{
    TexturePacker packer;

    std::vector<std::pair<std::string, Image>> data;
    data.emplace_back("solid", Image({16, 16}, {1.f, 1.f, 1.f, 1.f}, ColorFormat::RGBA));
    packer.pack(data);
    packer.create();

    EXPECT_NE(packer.get_texture().get_texture_handle(), 0u);

    TextureRegion region = packer.get_region("solid");
    EXPECT_GT(region.bounds.width, 0.f);
}
