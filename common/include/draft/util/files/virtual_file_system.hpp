#pragma once

#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/memory_file_provider.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace Draft {
    /**
     * @brief A filesystem that only ever talks to RAM, via MemoryFileProvider.
     *
     * This is meant mainly for testing.
     */
    class VirtualFileSystem {
    public:
        /**
         * @brief Resolves @p path to a disk-backed FileHandle. Does not require @p path to exist.
         */
        FileHandle open(const std::filesystem::path& path) const;

        /**
         * @brief Checks if @p path exists on disk.
         */
        bool exists(const std::filesystem::path& path) const;

        /**
         * @brief Creates the parent directories of @p path.
         */
        bool create_directories(const std::filesystem::path& path) const;

        /**
         * @brief Removes the file or directory at @p path.
         */
        bool remove(const std::filesystem::path& path) const;

        /**
         * @brief Copies the file or directory at @p from to @p to, creating @p to's parent
         * directories and overwriting existing files as needed.
         */
        void copy(const std::filesystem::path& from, const std::filesystem::path& to) const;

        /**
         * @brief Reads the entire contents of @p path as a string.
         */
        std::string read_string(const std::filesystem::path& path) const;

        /**
         * @brief Reads the contents of @p path as bytes, starting at @p offset.
         */
        std::vector<std::byte> read_bytes(const std::filesystem::path& path, std::size_t offset = 0) const;

        /**
         * @brief Writes @p contents to @p path.
         */
        void write_string(const std::filesystem::path& path, const std::string& contents) const;

        /**
         * @brief Writes @p bytes to @p path.
         */
        void write_bytes(const std::filesystem::path& path, const std::vector<std::byte>& bytes) const;

        /**
         * @brief Writes @p size bytes from @p data to @p path.
         */
        void write_bytes(const std::filesystem::path& path, const void* data, std::size_t size) const;

    private:
        MemoryFileProvider m_provider;
    };
}
