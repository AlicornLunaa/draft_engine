#pragma once

#include "draft/util/files/file_provider.hpp"
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <string>

namespace Draft {
    /**
     * @brief A FileProvider backed by the a virtual RAM filesystem.
     *
     * Paths are resolved relative to root, as its a virtual filesystem.
     * It supports reading and writing like disk, just not persistent.
     * 
     * Primary purpose is for testing.
     */
    class MemoryFileProvider final : public FileProvider {
    public:
        std::unique_ptr<FileProvider> clone() const override;

        bool exists(const std::filesystem::path& path) const override;
        bool is_directory(const std::filesystem::path& path) const override;
        bool is_file(const std::filesystem::path& path) const override;
        std::uintmax_t size(const std::filesystem::path& path) const override;
        Time last_modified(const std::filesystem::path& path) const override;

        bool remove(const std::filesystem::path& path) const override;
        bool create_directories(const std::filesystem::path& path) const override;

        std::string read_string(const std::filesystem::path& path) const override;
        void write_string(const std::filesystem::path& path, const std::string& str) const override;

        std::vector<std::byte> read_bytes(const std::filesystem::path& path, std::size_t offset) const override;
        void write_bytes(const std::filesystem::path& path, const void* data, std::size_t size) const override;

        std::vector<std::filesystem::path> list(const std::filesystem::path& path) const override;

        std::string get_absolute_path(const std::filesystem::path& path) const override;

    private:
        std::shared_ptr<std::unordered_map<std::string, std::string>> m_contents;
        std::shared_ptr<std::unordered_map<std::string, std::filesystem::file_time_type>> m_lastWriteTime;
    };
}
