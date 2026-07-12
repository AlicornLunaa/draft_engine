#pragma once

#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/file_provider.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Draft {
    /**
     * @brief Resolves asset paths against an ordered list of FileProvider backing stores.
     *
     * Providers are searched in the order they were added/registered; the first provider that
     * reports the path exists wins. The default constructor's search order is disk before
     * embedded, so a loose file on disk shadows the version baked into the binary.
     * 
     * Not writeable.
     */
    class AssetFileSystem {
    public:
        /**
         * @brief Constructs an AssetFileSystem with the default search order of disk then embedded.
         */
        AssetFileSystem();

        /**
         * @brief Constructs an AssetFileSystem searching @p providers in the given order.
         */
        explicit AssetFileSystem(std::vector<std::unique_ptr<FileProvider>> providers);

        /**
         * @brief Appends a clone of @p provider as the new lowest-precedence provider.
         */
        void add_provider(const FileProvider& provider);

        /**
         * @brief Checks whether @p path exists in any registered provider.
         */
        bool exists(const std::filesystem::path& path) const;

        /**
         * @brief Resolves @p path against the registered providers, in precedence order.
         * @throws std::runtime_error if @p path exists in none of them.
         */
        FileHandle open(const std::filesystem::path& path) const;

        /**
         * @brief Convenience for open(path).read_string().
         */
        std::string read_string(const std::filesystem::path& path) const;

        /**
         * @brief Convenience for open(path).read_bytes(offset).
         */
        std::vector<std::byte> read_bytes(const std::filesystem::path& path, std::size_t offset = 0) const;

    private:
        std::vector<std::unique_ptr<FileProvider>> m_providers;
    };
}
