#define GLFW_INCLUDE_NONE

#include <gtest/gtest.h>
#include "draft/asset/asset_manager.hpp"
#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/core/engine.hpp"
#include "draft/rendering/render_window.hpp"
#include "draft/util/files/asset_file_system.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/host_file_system.hpp"

#include "GLFW/glfw3.h"

#include <filesystem>

using namespace Draft;
namespace fs = std::filesystem;

TEST(AssetPipeline, ClassifyAssetByExtension)
{
    ASSERT_EQ(classify_asset("assets/textures/dev.png"), AssetKind::Texture);
    ASSERT_EQ(classify_asset("assets/textures/dev.JPG"), AssetKind::Texture); // case-insensitive
    ASSERT_EQ(classify_asset("assets/textures/dev.jpeg"), AssetKind::Texture);
    ASSERT_EQ(classify_asset("assets/fonts/default.ttf"), AssetKind::Font);
    ASSERT_EQ(classify_asset("assets/models/thing.glb"), AssetKind::Model);
    ASSERT_EQ(classify_asset("assets/models/thing.gltf"), AssetKind::Model);
    ASSERT_EQ(classify_asset("assets/sfx/boom.wav"), AssetKind::Sound);
    ASSERT_EQ(classify_asset("assets/sfx/boom.ogg"), AssetKind::Sound);
    ASSERT_EQ(classify_asset("assets/scenes/level1.json"), AssetKind::Scene);
    ASSERT_EQ(classify_asset("assets/scenes/dungeon/level2.json"), AssetKind::Scene);
    ASSERT_EQ(classify_asset("assets/shaders/default/vertex.glsl"), AssetKind::Unknown);
    ASSERT_EQ(classify_asset("assets/data.json"), AssetKind::Unknown); // .json outside assets/scenes/ isn't a Scene
}

class AssetPipelineProjectTest : public ::testing::Test {
protected:
    static fs::path projectRoot;

    static void SetUpTestSuite() {
        projectRoot = fs::absolute("test_asset_pipeline_project");
        fs::create_directories(projectRoot / "assets/textures");
        fs::create_directories(projectRoot / "assets/fonts");
        fs::create_directories(projectRoot / "assets/scenes");

        // Real font/texture bytes, not garbage
        AssetFileSystem sourceAssets;
        HostFileSystem hostFs;
        hostFs.write_bytes(projectRoot / "assets/fonts/default.ttf", sourceAssets.open("assets/fonts/default.ttf").read_bytes());
        hostFs.write_bytes(projectRoot / "assets/textures/debug_black.png", sourceAssets.open("assets/textures/debug_black.png").read_bytes());
        hostFs.write_string(projectRoot / "assets/scenes/level1.json", "{\"systems\":[],\"entities\":[]}");
        hostFs.write_string(projectRoot / "assets/shaders/ignored.glsl", "// not a validated type");

        glfwInit();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = new RenderWindow(64, 64, "asset_pipeline_test");
    }

    static void TearDownTestSuite() {
        delete window;
        window = nullptr;
        DiskFileProvider().remove(projectRoot);
    }

    static RenderWindow* window;
};

fs::path AssetPipelineProjectTest::projectRoot;
RenderWindow* AssetPipelineProjectTest::window = nullptr;

TEST_F(AssetPipelineProjectTest, CollectWalksAssetsRecursivelyAndSkipsUnknownExtensions)
{
    auto tasks = collect_project_assets(projectRoot);
    ASSERT_EQ(tasks.size(), 3u); // font, texture, scene - not the .glsl file

    bool foundFont = false, foundTexture = false, foundScene = false;
    for (const auto& task : tasks) {
        if (task.key == "assets/fonts/default.ttf") { foundFont = true; ASSERT_EQ(task.kind, AssetKind::Font); }
        if (task.key == "assets/textures/debug_black.png") { foundTexture = true; ASSERT_EQ(task.kind, AssetKind::Texture); }
        if (task.key == "assets/scenes/level1.json") { foundScene = true; ASSERT_EQ(task.kind, AssetKind::Scene); }
    }
    ASSERT_TRUE(foundFont);
    ASSERT_TRUE(foundTexture);
    ASSERT_TRUE(foundScene);
}

TEST_F(AssetPipelineProjectTest, ValidateAssetsSucceedsForARealProject)
{
    fs::path previousCwd = fs::current_path();
    fs::current_path(projectRoot);

    auto tasks = collect_project_assets(projectRoot);
    AssetManager assets;
    Engine sceneEngine;
    auto errors = validate_assets(assets, sceneEngine, tasks);

    fs::current_path(previousCwd);

    ASSERT_TRUE(errors.empty());
}

TEST_F(AssetPipelineProjectTest, ValidateAssetsReportsAFailingAssetWithoutAbortingTheRest)
{
    DiskFileProvider().write_string(projectRoot / "assets/textures/garbage.png", "not a real png");

    fs::path previousCwd = fs::current_path();
    fs::current_path(projectRoot);

    auto tasks = collect_project_assets(projectRoot);
    AssetManager assets;
    Engine sceneEngine;
    auto errors = validate_assets(assets, sceneEngine, tasks);

    fs::current_path(previousCwd);
    DiskFileProvider().remove(projectRoot / "assets/textures/garbage.png");

    ASSERT_EQ(errors.size(), 1u);
    ASSERT_TRUE(errors.contains("assets/textures/garbage.png"));
    // The other 3 real assets still validated even though one failed.
    ASSERT_EQ(tasks.size(), 4u);
}
