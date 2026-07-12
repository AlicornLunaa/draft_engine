#include <gtest/gtest.h>
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/embedded_file_provider.hpp"
#include <vector>
#include <string>
#include <filesystem>

using namespace Draft;

TEST(FileHandle, String)
{
    FileHandle handle = DiskFileProvider().open("test_file.txt");
    std::string content = "Hello, world!";
    handle.write_string(content);

    ASSERT_TRUE(handle.exists());
    ASSERT_EQ(handle.read_string(), content);
    handle.remove();
    ASSERT_FALSE(handle.exists());
}

TEST(FileHandle, Bytes)
{
    FileHandle handle = DiskFileProvider().open("test_file.bin");
    std::vector<std::byte> bytes = { (std::byte)0xDE, (std::byte)0xAD, (std::byte)0xBE, (std::byte)0xEF };
    handle.write_bytes(bytes);

    ASSERT_TRUE(handle.exists());
    std::vector<std::byte> read_bytes = handle.read_bytes();
    ASSERT_EQ(read_bytes.size(), bytes.size());
    for(size_t i = 0; i < bytes.size(); ++i)
    {
        ASSERT_EQ(read_bytes[i], bytes[i]);
    }
    handle.remove();
    ASSERT_FALSE(handle.exists());
}

TEST(FileHandle, PathManipulation)
{
    FileHandle dir = DiskFileProvider().open("test_dir");
    FileHandle file = dir / "subdir" / "file.txt";

    ASSERT_EQ(file.get_path(), "test_dir/subdir/file.txt");
    ASSERT_EQ(file.filename(), "file.txt");
    ASSERT_EQ(file.extension(), ".txt");
    ASSERT_EQ(file.stem(), "file");
    ASSERT_EQ(file.parent().get_path(), "test_dir/subdir");
}

TEST(FileHandle, Operators)
{
    FileHandle h1 = DiskFileProvider().open("base");
    FileHandle h2 = h1 / "sub";
    ASSERT_EQ(h2.get_path(), "base/sub");

    FileHandle h3 = h1 + "_suffix.ext";
    ASSERT_EQ(h3.get_path(), "base_suffix.ext");

    FileHandle h4 = h1;
    h4 /= "path";
    ASSERT_EQ(h4.get_path(), "base/path");

    FileHandle h5 = h1;
    h5 += ".tmp";
    ASSERT_EQ(h5.get_path(), "base.tmp");
}

TEST(FileHandle, ThrowsOnEmptyPath)
{
    ASSERT_THROW(DiskFileProvider().open(""), std::invalid_argument);
    ASSERT_THROW(FileHandle(std::filesystem::path(), DiskFileProvider()), std::invalid_argument);
}

TEST(FileHandle, EqualityDependsOnProviderKind)
{
    // Same path, same provider type, equal.
    ASSERT_EQ(DiskFileProvider().open("assets/shaders/default"), DiskFileProvider().open("assets/shaders/default"));

    // Same path, different provider types (disk vs embedded), not equal.
    ASSERT_NE(DiskFileProvider().open("assets/shaders/default"), EmbeddedFileProvider().open("assets/shaders/default"));
}

TEST(FileHandle, CopyIsIndependentAndUsable)
{
    // Copying a FileHandle must clone its provider rather than sharing/dangling it, so the
    // copy keeps working after the original goes out of scope.
    FileHandle copy = [] {
        FileHandle original = DiskFileProvider().open("test_copy_file.txt");
        return original;
    }();

    copy.write_string("copied");
    ASSERT_TRUE(copy.exists());
    ASSERT_EQ(copy.read_string(), "copied");
    copy.remove();
}

TEST(FileHandle, DirectoryListing)
{
    FileHandle dir = DiskFileProvider().open("test_list_dir");
    std::filesystem::create_directories("test_list_dir/sub");

    FileHandle f1 = dir / "file1.txt";
    f1.write_string("test");

    auto list = dir.list();
    ASSERT_GE(list.size(), 1);

    bool found = false;
    for(const auto& h : list) {
        if(h.filename() == "file1.txt") found = true;
    }
    ASSERT_TRUE(found);

    dir.remove();
}

TEST(FileHandle, EmbeddedProviderReadsBundledResource)
{
    // draft_common bundles assets/fonts/default.ttf via cmrc
    EmbeddedFileProvider provider;
    ASSERT_TRUE(provider.exists("assets/fonts/default.ttf"));
    ASSERT_GT(provider.size("assets/fonts/default.ttf"), 0u);

    FileHandle handle = provider.open("assets/fonts/default.ttf");
    ASSERT_TRUE(handle.exists());
    ASSERT_FALSE(handle.read_bytes().empty());
}

TEST(FileHandle, EmbeddedProviderIsReadOnly)
{
    EmbeddedFileProvider provider;
    ASSERT_THROW(provider.write_string("assets/fonts/default.ttf", "nope"), std::logic_error);
    ASSERT_THROW(provider.remove("assets/fonts/default.ttf"), std::logic_error);
}
