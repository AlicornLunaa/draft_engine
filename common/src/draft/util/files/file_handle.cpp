#include "draft/util/files/file_handle.hpp"

#include <stdexcept>
#include <typeinfo>

namespace fs = std::filesystem;

namespace Draft {
    // Constructors
    FileHandle::FileHandle(const fs::path& path, const FileProvider& provider) : m_path(path) {
        if (m_path.empty()) throw std::invalid_argument("FileHandle: path must not be empty");
        m_provider = provider.clone();
    }

    FileHandle::FileHandle(const FileHandle& other) : m_path(other.m_path), m_provider(other.m_provider->clone()) {}

    FileHandle& FileHandle::operator=(const FileHandle& other) {
        if (this == &other) return *this;
        m_path = other.m_path;
        m_provider = other.m_provider->clone();
        return *this;
    }

    // Path Information
    bool FileHandle::exists() const {
        return m_provider->exists(m_path);
    }

    bool FileHandle::is_directory() const {
        return m_provider->is_directory(m_path);
    }

    bool FileHandle::is_file() const {
        return m_provider->is_file(m_path);
    }

    std::uintmax_t FileHandle::size() const {
        return m_provider->size(m_path);
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
        return FileHandle(m_path.parent_path(), *m_provider);
    }

    std::string FileHandle::get_path() const {
        return m_path.generic_string();
    }

    std::string FileHandle::get_absolute_path() const {
        return m_provider->get_absolute_path(m_path);
    }

    Time FileHandle::last_modified() const {
        return m_provider->last_modified(m_path);
    }

    const FileProvider& FileHandle::get_provider() const {
        return *m_provider;
    }

    // File Operations
    bool FileHandle::remove() {
        return m_provider->remove(m_path);
    }

    bool FileHandle::create_directories() const {
        return m_provider->create_directories(m_path);
    }

    std::string FileHandle::read_string() const {
        return m_provider->read_string(m_path);
    }

    void FileHandle::write_string(const std::string& str) const {
        m_provider->write_string(m_path, str);
    }

    std::vector<std::byte> FileHandle::read_bytes(std::size_t offset) const {
        return m_provider->read_bytes(m_path, offset);
    }

    void FileHandle::write_bytes(const std::vector<std::byte>& bytes) const {
        write_bytes(bytes.data(), bytes.size());
    }

    void FileHandle::write_bytes(const void* data, std::size_t size) const {
        m_provider->write_bytes(m_path, data, size);
    }

    std::vector<FileHandle> FileHandle::list() const {
        std::vector<FileHandle> result;
        for (const auto& entryPath : m_provider->list(m_path)) {
            result.push_back(FileHandle(entryPath, *m_provider));
        }
        return result;
    }

    // Operators
    FileHandle FileHandle::operator/(const fs::path& other) const {
        return FileHandle(m_path / other, *m_provider);
    }

    FileHandle& FileHandle::operator/=(const fs::path& other) {
        m_path /= other;
        return *this;
    }

    FileHandle FileHandle::operator+(const std::string& other) const {
        return FileHandle(m_path.string() + other, *m_provider);
    }

    FileHandle& FileHandle::operator+=(const std::string& other) {
        m_path = m_path.string() + other;
        return *this;
    }

    bool FileHandle::operator==(const FileHandle& other) const {
        // Providers are stateless, so "the same kind of provider" (via typeid) is all that
        // distinguishes two handles beyond their path
        const FileProvider* lhs = m_provider.get();
        const FileProvider* rhs = other.m_provider.get();
        return m_path == other.m_path && typeid(*lhs) == typeid(*rhs);
    }

    bool FileHandle::operator!=(const FileHandle& other) const {
        return !(*this == other);
    }
}
