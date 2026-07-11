#include "draft/util/files/memory_file_provider.hpp"

#include <chrono>
#include <cstring>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    std::unique_ptr<FileProvider> MemoryFileProvider::clone() const {
        return std::make_unique<MemoryFileProvider>(*this);
    }

    bool MemoryFileProvider::exists(const fs::path& path) const {
        return m_contents->find(path) != m_contents->end();
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
        auto last_time = m_lastWriteTime->at(path);
        auto since_epoch = std::chrono::file_clock::to_sys(last_time).time_since_epoch();
        return Time::microseconds(std::chrono::duration_cast<std::chrono::microseconds>(since_epoch).count());
    }

    bool MemoryFileProvider::remove(const fs::path& path) const {
        return m_contents->erase(path) > 0;
    }

    bool MemoryFileProvider::create_directories(const fs::path& path) const {
        return true;
    }

    std::string MemoryFileProvider::read_string(const fs::path& path) const {
        bool in = (m_contents->find(path) != m_contents->end());
        if (!in) throw std::runtime_error("MemoryFileProvider: failed to open '" + path.string() + "' for reading");
        return m_contents->at(path);
    }

    void MemoryFileProvider::write_string(const fs::path& path, const std::string& str) const {
        (*m_contents)[path] = str;
        (*m_lastWriteTime)[path] = fs::file_time_type::clock::now();
    }

    std::vector<std::byte> MemoryFileProvider::read_bytes(const fs::path& path, std::size_t offset) const {
        auto it = m_contents->find(path);
        if (it == m_contents->end()) throw std::runtime_error("MemoryFileProvider: failed to open '" + path.string() + "' for reading");

        const std::string& contents = it->second;
        if (offset >= contents.size()) return {};

        std::size_t read_size = contents.size() - offset;
        std::vector<std::byte> buffer(read_size);
        std::memcpy(buffer.data(), contents.data() + offset, read_size);
        return buffer;
    }

    void MemoryFileProvider::write_bytes(const fs::path& path, const void* data, std::size_t size) const {
        (*m_contents)[path] = std::string(reinterpret_cast<const char*>(data), size);
        (*m_lastWriteTime)[path] = fs::file_time_type::clock::now();
    }

    std::vector<fs::path> MemoryFileProvider::list(const fs::path& path) const {
        std::vector<fs::path> result;
        for (const auto& [key, value] : *m_contents) {
            fs::path entry(key);
            if (entry.parent_path() == path) result.push_back(entry);
        }
        return result;
    }

    std::string MemoryFileProvider::get_absolute_path(const fs::path& path) const {
        return path.string();
    }
}
