#include <gtest/gtest.h>
#include "draft/util/file_handle.hpp"
#include <vector>
#include <string>

TEST(FileHandle, String)
{
    Draft::FileHandle handle("test_file.txt", Draft::FileHandle::LOCAL);
    std::string content = "Hello, world!";
    handle.write_string(content);

    ASSERT_TRUE(handle.exists());
    ASSERT_EQ(handle.read_string(), content + "\n");
    handle.remove();
    ASSERT_FALSE(handle.exists());
}

TEST(FileHandle, Bytes)
{
    Draft::FileHandle handle("test_file.bin", Draft::FileHandle::LOCAL);
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
