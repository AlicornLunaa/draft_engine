#include <gtest/gtest.h>
#include "draft/util/files/asset_file_system.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/embedded_file_provider.hpp"
#include "draft/util/files/memory_file_provider.hpp"
#include <memory>
#include <vector>

using namespace Draft;

TEST(AssetFileSystem, ExistsChecksAllProviders)
{
    AssetFileSystem fs;

    // assets/fonts/default.ttf is embedded (see common/CMakeLists.txt's RESOURCES) and not
    // present on disk relative to the test binary's working directory.
    ASSERT_TRUE(fs.exists("assets/fonts/default.ttf"));
    ASSERT_FALSE(fs.exists("definitely/not/a/real/path.xyz"));
}

TEST(AssetFileSystem, ReadsThroughToEmbeddedWhenNotOnDisk)
{
    AssetFileSystem fs;
    ASSERT_FALSE(fs.read_string("assets/fonts/default.ttf").empty());
}

TEST(AssetFileSystem, DiskTakesPrecedenceOverEmbedded)
{
    // "assets/fonts/default.ttf" is also embedded, so a matching entry earlier in the provider
    // list proves the search order actually prefers it rather than just happening to only ever
    // find one or the other. MemoryFileProvider stands in for "disk" here rather than actually
    // touching disk, since nothing about this test cares which real provider wins, only that
    // provider order is respected.
    auto embeddedProvider = std::make_unique<EmbeddedFileProvider>();
    auto memoryProvider = std::make_unique<MemoryFileProvider>();
    memoryProvider->open("assets/fonts/default.ttf").write_string("disk override");

    std::vector<std::unique_ptr<FileProvider>> vec;
    vec.push_back(std::move(memoryProvider));
    vec.push_back(std::move(embeddedProvider));

    AssetFileSystem fs(std::move(vec));
    ASSERT_EQ(fs.read_string("assets/fonts/default.ttf"), "disk override");
}

TEST(AssetFileSystem, ThrowsWhenNotFoundInAnyProvider)
{
    AssetFileSystem fs;
    ASSERT_THROW(fs.open("definitely/not/a/real/path.xyz"), std::runtime_error);
}

TEST(AssetFileSystem, CustomProviderListOnlySearchesGivenProviders)
{
    std::vector<std::unique_ptr<FileProvider>> providers;
    providers.push_back(std::make_unique<EmbeddedFileProvider>());
    AssetFileSystem fs(std::move(providers));

    ASSERT_TRUE(fs.exists("assets/fonts/default.ttf"));

    // Disk isn't in this provider list, so a disk-only file must not resolve.
    DiskFileProvider disk;
    disk.write_string("test_afs_disk_only.txt", "hi");
    ASSERT_FALSE(fs.exists("test_afs_disk_only.txt"));
    disk.remove("test_afs_disk_only.txt");
}
