#include "draft/util/files/memory_file_provider.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    std::unordered_map<std::string, std::string> MemoryFileProvider::s_contents{};
    std::unordered_map<std::string, std::filesystem::file_time_type> MemoryFileProvider::s_lastWriteTime{};

    std::unique_ptr<FileProvider> MemoryFileProvider::clone() const {
        return std::make_unique<MemoryFileProvider>(*this);
    }

    bool MemoryFileProvider::exists(const fs::path& path) const {
        return s_contents.find(path.string()) != s_contents.end();
    }

    bool MemoryFileProvider::is_directory(const fs::path& path) const {
        // Only stores files, no directory structure
        return true;
    }

    bool MemoryFileProvider::is_file(const fs::path& path) const {
        return exists(path);
    }

    std::uintmax_t MemoryFileProvider::size(const fs::path& path) const {
        return 0;
    }

    Time MemoryFileProvider::last_modified(const fs::path& path) const {
        auto last_time = s_lastWriteTime.at(path.string());
        auto since_epoch = std::chrono::file_clock::to_sys(last_time).time_since_epoch();
        return Time::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count());
    }

    bool MemoryFileProvider::remove(const fs::path& path) const {
        return s_contents.erase(path.string()) > 0;
    }

    bool MemoryFileProvider::create_directories(const fs::path& path) const {
        return true;
    }

    std::string MemoryFileProvider::read_string(const fs::path& path) const {
        bool in = (s_contents.find(path.string()) != s_contents.end());
        if (!in) throw std::runtime_error("MemoryFileProvider: failed to open '" + path.string() + "' for reading");
        return s_contents.at(path.string());
    }

    void MemoryFileProvider::write_string(const fs::path& path, const std::string& str) const {
        s_contents[path.string()] = str;
        s_lastWriteTime[path.string()] = fs::file_time_type::clock::now();
    }

    std::vector<std::byte> MemoryFileProvider::read_bytes(const fs::path& path, std::size_t offset) const {
        auto it = s_contents.find(path.string());
        if (it == s_contents.end()) throw std::runtime_error("MemoryFileProvider: failed to open '" + path.string() + "' for reading");

        const std::string& contents = it->second;
        if (offset >= contents.size()) return {};

        std::size_t read_size = contents.size() - offset;
        std::vector<std::byte> buffer(read_size);
        std::memcpy(buffer.data(), contents.data() + offset, read_size);
        return buffer;
    }

    void MemoryFileProvider::write_bytes(const fs::path& path, const void* data, std::size_t size) const {
        s_contents[path.string()] = std::string(reinterpret_cast<const char*>(data), size);
        s_lastWriteTime[path.string()] = fs::file_time_type::clock::now();
    }

    std::vector<fs::path> MemoryFileProvider::list(const fs::path& path) const {
        std::vector<fs::path> result;
        for (const auto& [key, value] : s_contents) {
            fs::path entry(key);
            if (entry.parent_path() == path) result.push_back(entry);
        }
        return result;
    }

    std::string MemoryFileProvider::get_absolute_path(const fs::path& path) const {
        return path.string();
    }
}
