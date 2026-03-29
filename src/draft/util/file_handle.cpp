#include "draft/util/file_handle.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

#include "cmrc/cmrc.hpp"

CMRC_DECLARE(draft_engine);

namespace fs = std::filesystem;

namespace Draft {
    // Constructors
    FileHandle::FileHandle() : m_path(""), m_access(INTERNAL) {}

    FileHandle::FileHandle(const fs::path& path, Access access) 
        : m_path(path), m_access(access) {}

    FileHandle::FileHandle(const std::string& path, Access access) 
        : m_path(path), m_access(access) {}

    FileHandle::FileHandle(const char* path, Access access) 
        : m_path(path), m_access(access) {}

    // Static factories
    FileHandle FileHandle::local(const fs::path& path) {
        return FileHandle(path, LOCAL);
    }

    FileHandle FileHandle::internal(const fs::path& path) {
        return FileHandle(path, INTERNAL);
    }

    FileHandle FileHandle::automatic(const std::string& path) {
        if (fs::exists(path)) {
            return FileHandle::local(path);
        }
        return FileHandle::internal(path);
    }

    // Path Information
    bool FileHandle::exists() const {
        if (!is_valid()) return false;

        if (m_access == LOCAL) {
            return fs::exists(m_path);
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            return internal_fs.exists(m_path.string());
        }
    }

    bool FileHandle::is_directory() const {
        if (!is_valid()) return false;

        if (m_access == LOCAL) {
            return fs::is_directory(m_path);
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            return internal_fs.is_directory(m_path.string());
        }
    }

    bool FileHandle::is_file() const {
        if (!is_valid()) return false;

        if (m_access == LOCAL) {
            return fs::is_regular_file(m_path);
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            return internal_fs.is_file(m_path.string());
        }
    }

    std::uintmax_t FileHandle::size() const {
        if (!is_valid()) return 0;

        if (m_access == LOCAL) {
            if (!fs::exists(m_path)) return 0;
            return fs::file_size(m_path);
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            try {
                auto file = internal_fs.open(m_path.string());
                return file.size();
            } catch (...) {
                return 0;
            }
        }
    }

    std::string FileHandle::extension() const {
        return m_path.extension().string();
    }

    std::string FileHandle::stem() const {
        return m_path.stem().string();
    }

    std::string FileHandle::filename() const {
        return m_path.filename().string();
    }

    FileHandle FileHandle::parent() const {
        return { m_path.parent_path(), m_access };
    }

    std::string FileHandle::get_path() const {
        return m_path.string();
    }

    std::string FileHandle::get_absolute_path() const {
        if (m_access == INTERNAL) return get_path();
        if (m_path.empty()) return "";
        return fs::absolute(m_path).string();
    }

    Time FileHandle::last_modified() const {
        if (m_access == INTERNAL || !fs::exists(m_path)) {
            return Time::microseconds(0);
        }
        auto last_time = fs::last_write_time(m_path);
        auto duration = last_time.time_since_epoch();
        return Time::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(duration).count());
    }

    bool FileHandle::is_valid() const {
        return !m_path.empty() && m_path != "null";
    }

    // File Operations
    bool FileHandle::remove() {
        if (m_access == INTERNAL || !is_valid()) return false;
        try {
            return fs::remove_all(m_path) > 0;
        } catch (...) {
            return false;
        }
    }

    bool FileHandle::create_directories() const {
        if (m_access == INTERNAL || !is_valid() || m_path.parent_path().empty()) return false;

        try {
            return fs::create_directories(m_path.parent_path());
        } catch (...) {
            return false;
        }
    }

    std::string FileHandle::read_string() const {
        if (!is_valid()) return "";

        if (m_access == LOCAL) {
            std::ifstream in(m_path, std::ios::in | std::ios::binary);
            if (!in) return "";
            std::string contents;
            in.seekg(0, std::ios::end);
            contents.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&contents[0], contents.size());
            return contents;
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            try {
                auto file = internal_fs.open(m_path.string());
                return std::string(file.begin(), file.end());
            } catch (...) {
                return "";
            }
        }
    }

    void FileHandle::write_string(const std::string& str) const {
        if (m_access == INTERNAL || !is_valid()) return;
        
        create_directories();
        std::ofstream out(m_path, std::ios::out | std::ios::binary);
        out << str;
    }

    std::vector<std::byte> FileHandle::read_bytes(std::size_t offset) const {
        if (!is_valid()) return {};

        if (m_access == LOCAL) {
            std::ifstream in(m_path, std::ios::binary);
            if (!in) return {};

            in.seekg(0, std::ios::end);
            std::size_t file_size = in.tellg();
            
            if (offset >= file_size) return {};
            
            std::size_t read_size = file_size - offset;
            std::vector<std::byte> buffer(read_size);
            
            in.seekg(offset, std::ios::beg);
            in.read(reinterpret_cast<char*>(buffer.data()), read_size);
            return buffer;
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            try {
                auto file = internal_fs.open(m_path.string());
                if (offset >= file.size()) return {};
                
                std::vector<std::byte> buffer;
                buffer.reserve(file.size() - offset);
                
                auto it = file.begin();
                std::advance(it, offset);
                
                for (; it != file.end(); ++it) {
                    buffer.push_back(static_cast<std::byte>(*it));
                }
                return buffer;
            } catch (...) {
                return {};
            }
        }
    }

    void FileHandle::write_bytes(const std::vector<std::byte>& bytes) const {
        write_bytes(bytes.data(), bytes.size());
    }

    void FileHandle::write_bytes(const void* data, std::size_t size) const {
        if (m_access == INTERNAL || !is_valid()) return;

        create_directories();
        std::ofstream out(m_path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(data), size);
    }

    std::vector<FileHandle> FileHandle::list() const {
        if (!is_valid() || !is_directory()) return {};

        std::vector<FileHandle> result;
        if (m_access == LOCAL) {
            for (const auto& entry : fs::directory_iterator(m_path)) {
                result.push_back(FileHandle::local(entry.path()));
            }
        } else {
            auto internal_fs = cmrc::draft_engine::get_filesystem();
            
            for (const auto& entry : internal_fs.iterate_directory(m_path.string())) {
                result.push_back(FileHandle::internal(m_path / entry.filename()));
            }
        }
        return result;
    }

    // Operators
    FileHandle FileHandle::operator/(const fs::path& other) const {
        return { m_path / other, m_access };
    }

    FileHandle& FileHandle::operator/=(const fs::path& other) {
        m_path /= other;
        return *this;
    }

    FileHandle FileHandle::operator+(const std::string& other) const {
        std::string new_path = m_path.string() + other;
        return { new_path, m_access };
    }

    FileHandle& FileHandle::operator+=(const std::string& other) {
        std::string new_path = m_path.string() + other;
        m_path = new_path;
        return *this;
    }

    bool FileHandle::operator==(const FileHandle& other) const {
        return m_access == other.m_access && m_path == other.m_path;
    }

    bool FileHandle::operator!=(const FileHandle& other) const {
        return !(*this == other);
    }
}
