#include "draft/util/files/virtual_file_system.hpp"

namespace fs = std::filesystem;

namespace Draft {
    FileHandle VirtualFileSystem::open(const fs::path& path) const {
        return m_provider.open(path);
    }

    bool VirtualFileSystem::exists(const fs::path& path) const {
        return m_provider.exists(path);
    }

    bool VirtualFileSystem::create_directories(const fs::path& path) const {
        return m_provider.create_directories(path);
    }

    bool VirtualFileSystem::remove(const fs::path& path) const {
        return m_provider.remove(path);
    }

    void VirtualFileSystem::copy(const fs::path& from, const fs::path& to) const {
        m_provider.write_string(to, m_provider.read_string(from));
    }

    std::string VirtualFileSystem::read_string(const fs::path& path) const {
        return m_provider.read_string(path);
    }

    std::vector<std::byte> VirtualFileSystem::read_bytes(const fs::path& path, std::size_t offset) const {
        return m_provider.read_bytes(path, offset);
    }

    void VirtualFileSystem::write_string(const fs::path& path, const std::string& contents) const {
        m_provider.write_string(path, contents);
    }

    void VirtualFileSystem::write_bytes(const fs::path& path, const std::vector<std::byte>& bytes) const {
        m_provider.write_bytes(path, bytes.data(), bytes.size());
    }

    void VirtualFileSystem::write_bytes(const fs::path& path, const void* data, std::size_t size) const {
        m_provider.write_bytes(path, data, size);
    }
}
