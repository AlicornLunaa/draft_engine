#pragma once

#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/file_provider.hpp"

#include <memory>

namespace Draft {
    /**
     * @brief A read-only FileProvider backed by a .apak, the format ApakWriter (build_tools) writes.
     *
     * The archive's bytes are read into memory once, via FileHandle, when this provider is
     * constructed, and its central directory indexed by path. Individual entries are only
     * decompressed on demand by read_string()/read_bytes()
     */
    class ArchiveFileProvider final : public FileProvider {
    public:
        /**
         * @brief Reads @p handle (expected to point at a .apak file) fully into memory and
         * indexes its central directory.
         * @throws std::runtime_error if @p handle doesn't point at a valid zip archive.
         */
        explicit ArchiveFileProvider(const FileHandle& handle);

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
        // Zip file index for @p path, throws std::runtime_error if it doesn't exist in the
        // archive or is a directory rather than a file.
        unsigned int locate_file(const std::filesystem::path& path) const;

        // pImpl: keeps miniz out of this header, and lets clone() just share the already-parsed
        // archive (shared_ptr copy) instead of re-reading/re-indexing the whole thing.
        struct Impl;
        std::shared_ptr<Impl> ptr;
    };
}
