#include <gtest/gtest.h>
#include "draft/util/files/embedded_file_provider.hpp"
#include "draft/util/files/file_handle.hpp"
#include <filesystem>

using namespace Draft;

TEST(EmbeddedFileProvider, MissingPathReportsAsNotExisting)
{
    EmbeddedFileProvider provider;
    ASSERT_FALSE(provider.exists("definitely/not/a/real/path.xyz"));
    ASSERT_FALSE(provider.is_file("definitely/not/a/real/path.xyz"));
    ASSERT_FALSE(provider.is_directory("definitely/not/a/real/path.xyz"));
}

TEST(EmbeddedFileProvider, FileAndDirectoryClassificationAreMutuallyExclusive)
{
    EmbeddedFileProvider provider;

    ASSERT_TRUE(provider.exists("assets/fonts/default.ttf"));
    ASSERT_TRUE(provider.is_file("assets/fonts/default.ttf"));
    ASSERT_FALSE(provider.is_directory("assets/fonts/default.ttf"));

    ASSERT_TRUE(provider.exists("assets/fonts"));
    ASSERT_TRUE(provider.is_directory("assets/fonts"));
    ASSERT_FALSE(provider.is_file("assets/fonts"));
}

TEST(EmbeddedFileProvider, Size)
{
    EmbeddedFileProvider provider;
    ASSERT_GT(provider.size("assets/fonts/default.ttf"), 0u);
}

TEST(EmbeddedFileProvider, SizeOfMissingResourceThrows)
{
    EmbeddedFileProvider provider;
    ASSERT_ANY_THROW(provider.size("definitely/not/a/real/path.xyz"));
}

TEST(EmbeddedFileProvider, LastModifiedIsAlwaysZero)
{
    // Documented limitation. cmrc bakes resources in at build time and keeps no timestamp
    // metadata, so this never throws and never varies - unlike DiskFileProvider's version.
    EmbeddedFileProvider provider;
    ASSERT_EQ(provider.last_modified("assets/fonts/default.ttf").as_microseconds(), 0);
}

TEST(EmbeddedFileProvider, StringRead)
{
    EmbeddedFileProvider provider;
    std::string contents = provider.read_string("assets/fonts/default.ttf");
    ASSERT_FALSE(contents.empty());
    ASSERT_EQ(contents.size(), provider.size("assets/fonts/default.ttf"));
}

TEST(EmbeddedFileProvider, ReadingMissingResourceThrows)
{
    EmbeddedFileProvider provider;
    ASSERT_ANY_THROW(provider.read_string("definitely/not/a/real/path.xyz"));
    ASSERT_ANY_THROW(provider.read_bytes("definitely/not/a/real/path.xyz", 0));
}

TEST(EmbeddedFileProvider, ReadBytesWithOffset)
{
    EmbeddedFileProvider provider;
    auto full = provider.read_bytes("assets/fonts/default.ttf", 0);
    auto tail = provider.read_bytes("assets/fonts/default.ttf", 4);

    ASSERT_EQ(tail.size(), full.size() - 4);
    for (size_t i = 0; i < tail.size(); ++i) {
        ASSERT_EQ(tail[i], full[i + 4]);
    }

    // Offset at/beyond the resource's size yields nothing, not an error.
    ASSERT_TRUE(provider.read_bytes("assets/fonts/default.ttf", full.size()).empty());
    ASSERT_TRUE(provider.read_bytes("assets/fonts/default.ttf", full.size() + 1000).empty());
}

TEST(EmbeddedFileProvider, MutatingOperationsAreRejected)
{
    EmbeddedFileProvider provider;
    ASSERT_THROW(provider.write_string("assets/fonts/default.ttf", "nope"), std::logic_error);
    ASSERT_THROW(provider.write_bytes("assets/fonts/default.ttf", "no", 2), std::logic_error);
    ASSERT_THROW(provider.remove("assets/fonts/default.ttf"), std::logic_error);
    ASSERT_THROW(provider.create_directories("assets/fonts/default.ttf"), std::logic_error);
}

TEST(EmbeddedFileProvider, List)
{
    EmbeddedFileProvider provider;
    auto entries = provider.list("assets/fonts");
    ASSERT_EQ(entries.size(), 3u);

    bool found_default = false, found_arial = false, found_missing = false;
    for (const auto& entry : entries) {
        if (entry.filename() == "default.ttf") found_default = true;
        if (entry.filename() == "arial.ttf") found_arial = true;
        if (entry.filename() == "missing_font.ttf") found_missing = true;
    }
    ASSERT_TRUE(found_default);
    ASSERT_TRUE(found_arial);
    ASSERT_TRUE(found_missing);
}

TEST(EmbeddedFileProvider, ListOfMissingOrNonDirectoryPathIsEmpty)
{
    EmbeddedFileProvider provider;
    ASSERT_TRUE(provider.list("definitely/not/a/real/path").empty());
    ASSERT_TRUE(provider.list("assets/fonts/default.ttf").empty());
}

TEST(EmbeddedFileProvider, GetAbsolutePathReturnsPathUnchanged)
{
    // Embedded resources have no real filesystem location.
    EmbeddedFileProvider provider;
    ASSERT_EQ(provider.get_absolute_path("assets/fonts/default.ttf"), "assets/fonts/default.ttf");
}

TEST(EmbeddedFileProvider, OpenCreatesWorkingFileHandle)
{
    EmbeddedFileProvider provider;
    FileHandle handle = provider.open("assets/fonts/default.ttf");

    ASSERT_TRUE(handle.exists());
    ASSERT_FALSE(handle.read_bytes().empty());
}

TEST(EmbeddedFileProvider, CloneIsIndependentAndFunctional)
{
    EmbeddedFileProvider provider;
    std::unique_ptr<FileProvider> clone = provider.clone();

    ASSERT_NE(clone, nullptr);
    ASSERT_NE(dynamic_cast<EmbeddedFileProvider*>(clone.get()), nullptr);
    ASSERT_TRUE(clone->exists("assets/fonts/default.ttf"));
}
