#include <gtest/gtest.h>
#include "draft/util/files/host_file_system.hpp"
#include <vector>

using namespace Draft;

TEST(HostFileSystem, StringReadWrite)
{
    HostFileSystem fs;
    fs.write_string("test_hfs_file.txt", "hello");

    ASSERT_TRUE(fs.exists("test_hfs_file.txt"));
    ASSERT_EQ(fs.read_string("test_hfs_file.txt"), "hello");

    fs.remove("test_hfs_file.txt");
    ASSERT_FALSE(fs.exists("test_hfs_file.txt"));
}

TEST(HostFileSystem, BytesReadWrite)
{
    HostFileSystem fs;
    std::vector<std::byte> bytes = { (std::byte)0x01, (std::byte)0x02, (std::byte)0x03 };
    fs.write_bytes("test_hfs_file.bin", bytes);

    auto read_back = fs.read_bytes("test_hfs_file.bin");
    ASSERT_EQ(read_back.size(), bytes.size());
    for (size_t i = 0; i < bytes.size(); ++i) {
        ASSERT_EQ(read_back[i], bytes[i]);
    }

    fs.remove("test_hfs_file.bin");
}

TEST(HostFileSystem, Open)
{
    HostFileSystem fs;
    FileHandle handle = fs.open("test_hfs_open.txt");
    handle.write_string("via handle");

    ASSERT_EQ(fs.read_string("test_hfs_open.txt"), "via handle");
    fs.remove("test_hfs_open.txt");
}

TEST(HostFileSystem, CreateDirectories)
{
    HostFileSystem fs;
    ASSERT_TRUE(fs.create_directories("test_hfs_dir/nested/file.txt"));
    ASSERT_TRUE(fs.exists("test_hfs_dir/nested"));

    fs.remove("test_hfs_dir");
}

TEST(HostFileSystem, Copy)
{
    HostFileSystem fs;
    fs.write_string("test_hfs_src.txt", "copy me");
    fs.copy("test_hfs_src.txt", "test_hfs_dst_dir/copied.txt");

    ASSERT_TRUE(fs.exists("test_hfs_dst_dir/copied.txt"));
    ASSERT_EQ(fs.read_string("test_hfs_dst_dir/copied.txt"), "copy me");

    fs.remove("test_hfs_src.txt");
    fs.remove("test_hfs_dst_dir");
}
