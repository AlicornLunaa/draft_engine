#include <gtest/gtest.h>
#include "draft/build_tools/apak_writer.hpp"
#include "draft/util/files/archive_file_provider.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/host_file_system.hpp"

#include <filesystem>
#include <stdexcept>

using namespace Draft;
namespace fs = std::filesystem;

namespace {
    void make_fixture_tree(const fs::path& root) {
        HostFileSystem hostFs;
        hostFs.write_string(root / "assets/textures/dev.png", "texture-bytes");
        hostFs.write_string(root / "assets/fonts/default.ttf", "font-bytes");
        hostFs.write_string(root / "assets/scenes/level1.json", "{\"ok\":true}");
    }
}

class ApakWriterTest : public ::testing::Test {
protected:
    static fs::path sourceDir;
    static fs::path outputPath;

    static void SetUpTestSuite() {
        sourceDir = "test_apak_writer_source";
        outputPath = "test_apak_writer_output.apak";
        make_fixture_tree(sourceDir);
    }

    static void TearDownTestSuite() {
        DiskFileProvider().remove(sourceDir);
        DiskFileProvider().remove(outputPath);
    }
};

fs::path ApakWriterTest::sourceDir;
fs::path ApakWriterTest::outputPath;

TEST_F(ApakWriterTest, WritingMissingSourceDirThrows)
{
    ASSERT_THROW(ApakWriter::write("test_apak_writer_does_not_exist", "test_apak_writer_never.apak"), std::runtime_error);
    ASSERT_FALSE(DiskFileProvider().exists("test_apak_writer_never.apak"));
}

TEST_F(ApakWriterTest, RoundTripsEveryFileWithSourceRelativeForwardSlashedNames)
{
    ApakWriter::write(sourceDir, outputPath);
    ASSERT_TRUE(DiskFileProvider().exists(outputPath));

    // Read the result back through the exact provider AssetFileSystem would use for a real .apak, not just the raw zip API
    ArchiveFileProvider provider(DiskFileProvider().open(outputPath));

    ASSERT_TRUE(provider.is_file("assets/textures/dev.png"));
    ASSERT_EQ(provider.read_string("assets/textures/dev.png"), "texture-bytes");

    ASSERT_TRUE(provider.is_file("assets/fonts/default.ttf"));
    ASSERT_EQ(provider.read_string("assets/fonts/default.ttf"), "font-bytes");

    ASSERT_TRUE(provider.is_file("assets/scenes/level1.json"));
    ASSERT_EQ(provider.read_string("assets/scenes/level1.json"), "{\"ok\":true}");

    // Directories aren't explicit entries here (HostFileSystem::write_string never adds one),
    // so this also covers ArchiveFileProvider inferring them from the files' own paths.
    ASSERT_TRUE(provider.is_directory("assets/textures"));
    ASSERT_TRUE(provider.is_directory("assets"));
}

TEST_F(ApakWriterTest, WritingToAnExistingOutputPathOverwritesItCleanly)
{
    ApakWriter::write(sourceDir, outputPath);
    ApakWriter::write(sourceDir, outputPath);

    ArchiveFileProvider provider(DiskFileProvider().open(outputPath));
    ASSERT_EQ(provider.read_string("assets/fonts/default.ttf"), "font-bytes");
}
