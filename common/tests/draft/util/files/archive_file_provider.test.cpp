#include <gtest/gtest.h>
#include "draft/util/files/archive_file_provider.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/file_handle.hpp"

#include "miniz.h"

#include <filesystem>
#include <stdexcept>
#include <vector>

using namespace Draft;

namespace {
    // Writes a small real .apak directly via miniz's writer API, independent of ApakWriter
    // (build_tools), which Common can't depend on - this keeps ArchiveFileProvider testable in
    // isolation from the layer that packs real projects into archives.
    void write_test_apak(const std::filesystem::path& path) {
        mz_zip_archive zip{};
        ASSERT_TRUE(mz_zip_writer_init_file(&zip, path.string().c_str(), 0));

        const char* fontData = "font-bytes";
        const char* textureData = "texture-bytes";
        const char* sceneData = "{\"ok\":true}";

        ASSERT_TRUE(mz_zip_writer_add_mem(&zip, "assets/fonts/default.ttf", fontData, 10, MZ_DEFAULT_COMPRESSION));
        ASSERT_TRUE(mz_zip_writer_add_mem(&zip, "assets/textures/dev.png", textureData, 13, MZ_DEFAULT_COMPRESSION));
        ASSERT_TRUE(mz_zip_writer_add_mem(&zip, "assets/scenes/level1.json", sceneData, 11, MZ_DEFAULT_COMPRESSION));
        // An explicit (empty) directory entry, to cover archives that do include them, not just
        // the directories ArchiveFileProvider infers from file paths.
        ASSERT_TRUE(mz_zip_writer_add_mem(&zip, "assets/empty/", nullptr, 0, MZ_DEFAULT_COMPRESSION));

        ASSERT_TRUE(mz_zip_writer_finalize_archive(&zip));
        ASSERT_TRUE(mz_zip_writer_end(&zip));
    }
}

class ArchiveFileProviderTest : public ::testing::Test {
protected:
    static std::filesystem::path apakPath;

    static void SetUpTestSuite() {
        apakPath = "test_afp_fixture.apak";
        write_test_apak(apakPath);
    }

    static void TearDownTestSuite() {
        DiskFileProvider().remove(apakPath);
    }

    static ArchiveFileProvider open() {
        return ArchiveFileProvider(DiskFileProvider().open(apakPath));
    }
};

std::filesystem::path ArchiveFileProviderTest::apakPath;

TEST_F(ArchiveFileProviderTest, MissingPathReportsAsNotExisting)
{
    ArchiveFileProvider provider = open();
    ASSERT_FALSE(provider.exists("assets/does_not_exist.png"));
    ASSERT_FALSE(provider.is_file("assets/does_not_exist.png"));
    ASSERT_FALSE(provider.is_directory("assets/does_not_exist.png"));
}

TEST_F(ArchiveFileProviderTest, FileAndDirectoryClassificationAreMutuallyExclusive)
{
    ArchiveFileProvider provider = open();

    ASSERT_TRUE(provider.is_file("assets/fonts/default.ttf"));
    ASSERT_FALSE(provider.is_directory("assets/fonts/default.ttf"));

    // "assets/fonts" has no explicit zip entry, it's inferred from default.ttf's path.
    ASSERT_TRUE(provider.is_directory("assets/fonts"));
    ASSERT_FALSE(provider.is_file("assets/fonts"));

    // "assets/empty" does have an explicit zip directory entry.
    ASSERT_TRUE(provider.is_directory("assets/empty"));
    ASSERT_FALSE(provider.is_file("assets/empty"));
}

TEST_F(ArchiveFileProviderTest, Size)
{
    ArchiveFileProvider provider = open();
    ASSERT_EQ(provider.size("assets/fonts/default.ttf"), 10u);
    ASSERT_EQ(provider.size("assets/textures/dev.png"), 13u);
}

TEST_F(ArchiveFileProviderTest, SizeOfMissingOrDirectoryPathThrows)
{
    ArchiveFileProvider provider = open();
    ASSERT_THROW(provider.size("assets/does_not_exist.png"), std::runtime_error);
    ASSERT_THROW(provider.size("assets/fonts"), std::runtime_error);
}

TEST_F(ArchiveFileProviderTest, LastModifiedDoesNotThrow)
{
    // The fixture is written via mz_zip_writer_add_mem(), which doesn't record a real disk
    // mtime the way ApakWriter's mz_zip_writer_add_file() does - just check it's callable and
    // doesn't throw for a real entry.
    ArchiveFileProvider provider = open();
    ASSERT_NO_THROW(provider.last_modified("assets/fonts/default.ttf"));
}

TEST_F(ArchiveFileProviderTest, StringRead)
{
    ArchiveFileProvider provider = open();
    ASSERT_EQ(provider.read_string("assets/scenes/level1.json"), "{\"ok\":true}");
}

TEST_F(ArchiveFileProviderTest, ReadingMissingOrDirectoryPathThrows)
{
    ArchiveFileProvider provider = open();
    ASSERT_THROW(provider.read_string("assets/does_not_exist.png"), std::runtime_error);
    ASSERT_THROW(provider.read_bytes("assets/fonts", 0), std::runtime_error);
}

TEST_F(ArchiveFileProviderTest, ReadBytesWithOffset)
{
    ArchiveFileProvider provider = open();
    auto tail = provider.read_bytes("assets/fonts/default.ttf", 5);
    ASSERT_EQ(tail.size(), 5u);
    ASSERT_EQ(static_cast<char>(tail[0]), 'b'); // "font-bytes"[5..] == "bytes"

    // Offset at/beyond the end of the entry yields nothing, not an error.
    ASSERT_TRUE(provider.read_bytes("assets/fonts/default.ttf", 10).empty());
    ASSERT_TRUE(provider.read_bytes("assets/fonts/default.ttf", 1000).empty());
}

TEST_F(ArchiveFileProviderTest, MutatingOperationsAreRejected)
{
    ArchiveFileProvider provider = open();
    ASSERT_THROW(provider.write_string("assets/fonts/default.ttf", "nope"), std::logic_error);
    ASSERT_THROW(provider.write_bytes("assets/fonts/default.ttf", "no", 2), std::logic_error);
    ASSERT_THROW(provider.remove("assets/fonts/default.ttf"), std::logic_error);
    ASSERT_THROW(provider.create_directories("assets/fonts/default.ttf"), std::logic_error);
}

TEST_F(ArchiveFileProviderTest, ListReturnsDirectChildrenOnly)
{
    ArchiveFileProvider provider = open();
    auto entries = provider.list("assets");
    ASSERT_EQ(entries.size(), 4u); // fonts/, textures/, scenes/, empty/ - not the files nested inside them

    bool foundFonts = false, foundTextures = false, foundScenes = false, foundEmpty = false;
    for (const auto& entry : entries) {
        if (entry.filename() == "fonts") foundFonts = true;
        if (entry.filename() == "textures") foundTextures = true;
        if (entry.filename() == "scenes") foundScenes = true;
        if (entry.filename() == "empty") foundEmpty = true;
    }
    ASSERT_TRUE(foundFonts);
    ASSERT_TRUE(foundTextures);
    ASSERT_TRUE(foundScenes);
    ASSERT_TRUE(foundEmpty);
}

TEST_F(ArchiveFileProviderTest, ListOfMissingOrNonDirectoryPathIsEmpty)
{
    ArchiveFileProvider provider = open();
    ASSERT_TRUE(provider.list("definitely/not/real").empty());
    ASSERT_TRUE(provider.list("assets/fonts/default.ttf").empty());
}

TEST_F(ArchiveFileProviderTest, GetAbsolutePathIncludesTheArchivesOwnLocation)
{
    ArchiveFileProvider provider = open();
    std::string absolute = provider.get_absolute_path("assets/fonts/default.ttf");

    ASSERT_NE(absolute.find("assets/fonts/default.ttf"), std::string::npos);
    ASSERT_NE(absolute.find(apakPath.filename().string()), std::string::npos);
}

TEST_F(ArchiveFileProviderTest, OpenCreatesWorkingFileHandle)
{
    ArchiveFileProvider provider = open();
    FileHandle handle = provider.open("assets/fonts/default.ttf");

    ASSERT_TRUE(handle.exists());
    ASSERT_EQ(handle.read_string(), "font-bytes");
}

TEST_F(ArchiveFileProviderTest, CloneIsIndependentAndFunctional)
{
    ArchiveFileProvider provider = open();
    std::unique_ptr<FileProvider> clone = provider.clone();

    ASSERT_NE(clone, nullptr);
    ASSERT_NE(dynamic_cast<ArchiveFileProvider*>(clone.get()), nullptr);
    ASSERT_EQ(clone->read_string("assets/fonts/default.ttf"), "font-bytes");
}

TEST(ArchiveFileProvider, ConstructingFromNonZipDataThrows)
{
    DiskFileProvider().write_string("test_afp_garbage.apak", "not a zip file");
    ASSERT_THROW(ArchiveFileProvider provider(DiskFileProvider().open("test_afp_garbage.apak")), std::runtime_error);
    DiskFileProvider().remove("test_afp_garbage.apak");
}
