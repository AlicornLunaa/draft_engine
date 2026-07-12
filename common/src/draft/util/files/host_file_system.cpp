#include "draft/util/files/host_file_system.hpp"

namespace fs = std::filesystem;

namespace Draft {
    FileHandle HostFileSystem::open(const fs::path& path) const {
        return m_provider.open(path);
    }

    bool HostFileSystem::exists(const fs::path& path) const {
        return m_provider.exists(path);
    }

    bool HostFileSystem::create_directories(const fs::path& path) const {
        return m_provider.create_directories(path);
    }

    bool HostFileSystem::remove(const fs::path& path) const {
        return m_provider.remove(path);
    }

    void HostFileSystem::copy(const fs::path& from, const fs::path& to) const {
        if (!to.parent_path().empty()) fs::create_directories(to.parent_path());

        // Left to throw fs::filesystem_error on I/O failure
        fs::copy(from, to, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
    }

    std::string HostFileSystem::read_string(const fs::path& path) const {
        return m_provider.read_string(path);
    }

    std::vector<std::byte> HostFileSystem::read_bytes(const fs::path& path, std::size_t offset) const {
        return m_provider.read_bytes(path, offset);
    }

    void HostFileSystem::write_string(const fs::path& path, const std::string& contents) const {
        m_provider.write_string(path, contents);
    }

    void HostFileSystem::write_bytes(const fs::path& path, const std::vector<std::byte>& bytes) const {
        m_provider.write_bytes(path, bytes.data(), bytes.size());
    }

    void HostFileSystem::write_bytes(const fs::path& path, const void* data, std::size_t size) const {
        m_provider.write_bytes(path, data, size);
    }
}
