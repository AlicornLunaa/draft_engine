#include <gtest/gtest.h>
#include "draft/util/files/disk_file_provider.hpp"
#include "draft/util/files/file_handle.hpp"
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <vector>

using namespace Draft;

TEST(DiskFileProvider, MissingPathReportsAsNotExisting)
{
    DiskFileProvider provider;
    ASSERT_FALSE(provider.exists("test_dfp_does_not_exist.txt"));
    ASSERT_FALSE(provider.is_file("test_dfp_does_not_exist.txt"));
    ASSERT_FALSE(provider.is_directory("test_dfp_does_not_exist.txt"));
}

TEST(DiskFileProvider, StringReadWrite)
{
    DiskFileProvider provider;
    provider.write_string("test_dfp_string.txt", "hello disk");

    ASSERT_TRUE(provider.exists("test_dfp_string.txt"));
    ASSERT_TRUE(provider.is_file("test_dfp_string.txt"));
    ASSERT_FALSE(provider.is_directory("test_dfp_string.txt"));
    ASSERT_EQ(provider.read_string("test_dfp_string.txt"), "hello disk");
    ASSERT_EQ(provider.size("test_dfp_string.txt"), std::string("hello disk").size());

    provider.remove("test_dfp_string.txt");
    ASSERT_FALSE(provider.exists("test_dfp_string.txt"));
}

TEST(DiskFileProvider, BytesReadWrite)
{
    DiskFileProvider provider;
    std::vector<std::byte> bytes = { (std::byte)0xCA, (std::byte)0xFE, (std::byte)0xBA, (std::byte)0xBE };
    provider.write_bytes("test_dfp_bytes.bin", bytes.data(), bytes.size());

    auto read_back = provider.read_bytes("test_dfp_bytes.bin", 0);
    ASSERT_EQ(read_back.size(), bytes.size());
    for (size_t i = 0; i < bytes.size(); ++i) {
        ASSERT_EQ(read_back[i], bytes[i]);
    }

    provider.remove("test_dfp_bytes.bin");
}

TEST(DiskFileProvider, ReadBytesWithOffset)
{
    DiskFileProvider provider;
    provider.write_string("test_dfp_offset.txt", "0123456789");

    auto tail = provider.read_bytes("test_dfp_offset.txt", 5);
    ASSERT_EQ(tail.size(), 5u);
    ASSERT_EQ(static_cast<char>(tail[0]), '5');

    // Offset at/beyond the end of the file yields nothing, not an error.
    ASSERT_TRUE(provider.read_bytes("test_dfp_offset.txt", 10).empty());
    ASSERT_TRUE(provider.read_bytes("test_dfp_offset.txt", 1000).empty());

    provider.remove("test_dfp_offset.txt");
}

TEST(DiskFileProvider, ReadingMissingFileThrows)
{
    DiskFileProvider provider;
    ASSERT_THROW(provider.read_string("test_dfp_missing_read.txt"), std::runtime_error);
    ASSERT_THROW(provider.read_bytes("test_dfp_missing_read.txt", 0), std::runtime_error);
}

TEST(DiskFileProvider, SizeAndLastModifiedThrowForMissingFile)
{
    DiskFileProvider provider;
    ASSERT_THROW(provider.size("test_dfp_missing_meta.txt"), std::filesystem::filesystem_error);
    ASSERT_THROW(provider.last_modified("test_dfp_missing_meta.txt"), std::filesystem::filesystem_error);
}

TEST(DiskFileProvider, LastModifiedMatchesWallClockAtTimeOfWrite)
{
    auto before = std::chrono::system_clock::now();
    DiskFileProvider provider;
    provider.write_string("test_dfp_mtime.txt", "x");
    auto after = std::chrono::system_clock::now();

    int64_t modified_us = provider.last_modified("test_dfp_mtime.txt").as_microseconds();
    int64_t before_us = std::chrono::duration_cast<std::chrono::microseconds>(before.time_since_epoch()).count();
    int64_t after_us = std::chrono::duration_cast<std::chrono::microseconds>(after.time_since_epoch()).count();

    // Generous slack for filesystem mtime resolution/clock drift, not tight timing precision.
    const int64_t slack_us = 5'000'000;
    ASSERT_GE(modified_us, before_us - slack_us);
    ASSERT_LE(modified_us, after_us + slack_us);

    provider.remove("test_dfp_mtime.txt");
}

TEST(DiskFileProvider, RemoveOfMissingPathReturnsFalse)
{
    DiskFileProvider provider;
    ASSERT_FALSE(provider.remove("test_dfp_never_existed.txt"));
}

TEST(DiskFileProvider, CreateDirectoriesMakesParentOfPath)
{
    DiskFileProvider provider;
    ASSERT_TRUE(provider.create_directories("test_dfp_dir/nested/file.txt"));
    ASSERT_TRUE(provider.is_directory("test_dfp_dir/nested"));

    provider.remove("test_dfp_dir");
}

TEST(DiskFileProvider, CreateDirectoriesWithNoParentReturnsFalse)
{
    DiskFileProvider provider;
    ASSERT_FALSE(provider.create_directories("bare_name_with_no_directory.txt"));
}

TEST(DiskFileProvider, List)
{
    DiskFileProvider provider;
    provider.create_directories("test_dfp_list/a.txt");
    provider.write_string("test_dfp_list/a.txt", "a");
    provider.write_string("test_dfp_list/b.txt", "b");

    auto entries = provider.list("test_dfp_list");
    ASSERT_EQ(entries.size(), 2u);

    bool found_a = false, found_b = false;
    for (const auto& entry : entries) {
        if (entry.filename() == "a.txt") found_a = true;
        if (entry.filename() == "b.txt") found_b = true;
    }
    ASSERT_TRUE(found_a);
    ASSERT_TRUE(found_b);

    provider.remove("test_dfp_list");
}

TEST(DiskFileProvider, ListOfMissingOrNonDirectoryPathIsEmpty)
{
    DiskFileProvider provider;
    ASSERT_TRUE(provider.list("test_dfp_does_not_exist_dir").empty());

    provider.write_string("test_dfp_not_a_dir.txt", "x");
    ASSERT_TRUE(provider.list("test_dfp_not_a_dir.txt").empty());
    provider.remove("test_dfp_not_a_dir.txt");
}

TEST(DiskFileProvider, GetAbsolutePath)
{
    DiskFileProvider provider;
    ASSERT_EQ(provider.get_absolute_path(""), "");

    std::string absolute = provider.get_absolute_path("test_dfp_relative.txt");
    ASSERT_TRUE(std::filesystem::path(absolute).is_absolute());
}

TEST(DiskFileProvider, OpenCreatesWorkingFileHandle)
{
    DiskFileProvider provider;
    FileHandle handle = provider.open("test_dfp_open.txt");
    handle.write_string("via provider.open");

    ASSERT_EQ(provider.read_string("test_dfp_open.txt"), "via provider.open");
    provider.remove("test_dfp_open.txt");
}

TEST(DiskFileProvider, CloneIsIndependentAndFunctional)
{
    DiskFileProvider provider;
    std::unique_ptr<FileProvider> clone = provider.clone();

    ASSERT_NE(clone, nullptr);
    ASSERT_NE(dynamic_cast<DiskFileProvider*>(clone.get()), nullptr);

    clone->write_string("test_dfp_clone.txt", "from clone");
    ASSERT_EQ(provider.read_string("test_dfp_clone.txt"), "from clone");

    provider.remove("test_dfp_clone.txt");
}
