#include <gtest/gtest.h>
#include "draft/util/file_handle.hpp"
#include <vector>
#include <string>
#include <filesystem>

using namespace Draft;

TEST(FileHandle, String)
{
    FileHandle handle("test_file.txt", FileHandle::LOCAL);
    std::string content = "Hello, world!";
    handle.write_string(content);

    ASSERT_TRUE(handle.exists());
    ASSERT_EQ(handle.read_string(), content);
    handle.remove();
    ASSERT_FALSE(handle.exists());
}

TEST(FileHandle, Bytes)
{
    FileHandle handle("test_file.bin", FileHandle::LOCAL);
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
    FileHandle dir("test_dir", FileHandle::LOCAL);
    FileHandle file = dir / "subdir" / "file.txt";
    
    ASSERT_EQ(file.get_path(), "test_dir/subdir/file.txt");
    ASSERT_EQ(file.filename(), "file.txt");
    ASSERT_EQ(file.extension(), ".txt");
    ASSERT_EQ(file.stem(), "file");
    ASSERT_EQ(file.parent().get_path(), "test_dir/subdir");
}

TEST(FileHandle, Operators)
{
    FileHandle h1("base", FileHandle::LOCAL);
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

TEST(FileHandle, Validity)
{
    FileHandle invalid;
    ASSERT_FALSE(invalid);
    ASSERT_FALSE(invalid.is_valid());
    
    FileHandle valid("path");
    ASSERT_TRUE(valid);
    ASSERT_TRUE(valid.is_valid());
}

TEST(FileHandle, DirectoryListing)
{
    FileHandle dir("test_list_dir", FileHandle::LOCAL);
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
