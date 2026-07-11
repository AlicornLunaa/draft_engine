#pragma once

#include "draft/util/time.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Draft {
    class FileHandle;

    /**
     * @brief Abstract backing store for FileHandle.
     *
     * A FileProvider knows how to answer every filesystem-shaped question (existence,
     * size, reads, writes, directory listings, ...) for paths relative to whatever
     * store it represents. FileHandle owns a clone of one of these and forwards every
     * operation to it, so new storage backends (disk, embedded resources, an archive,
     * a network share, ...) can be added by implementing this interface rather than by
     * extending FileHandle itself.
     *
     * Providers are expected to be cheap, value-like objects (see clone()); FileHandle
     * relies on that to give itself ordinary value semantics without reaching for a
     * shared/global provider instance.
     */
    class FileProvider {
    public:
        virtual ~FileProvider() = default;

        /**
         * @brief Creates an owned copy of this provider.
         * Lets FileHandle hold a self-contained provider without assuming anything about
         * where the original instance lives (stack, member field, temporary, ...).
         */
        virtual std::unique_ptr<FileProvider> clone() const = 0;

        /**
         * @brief Checks whether @p path exists in this provider.
         */
        virtual bool exists(const std::filesystem::path& path) const = 0;

        /**
         * @brief Checks whether @p path refers to a directory.
         */
        virtual bool is_directory(const std::filesystem::path& path) const = 0;

        /**
         * @brief Checks whether @p path refers to a regular file.
         */
        virtual bool is_file(const std::filesystem::path& path) const = 0;

        /**
         * @brief Gets the size of the file at @p path, in bytes.
         */
        virtual std::uintmax_t size(const std::filesystem::path& path) const = 0;

        /**
         * @brief Gets the last modification time of the file at @p path.
         */
        virtual Time last_modified(const std::filesystem::path& path) const = 0;

        /**
         * @brief Removes the file or directory at @p path, if this provider supports mutation.
         * @return True if something was removed.
         */
        virtual bool remove(const std::filesystem::path& path) const = 0;

        /**
         * @brief Ensures the parent directories of @p path exist, if this provider supports mutation.
         * @return True if the directories exist afterwards.
         */
        virtual bool create_directories(const std::filesystem::path& path) const = 0;

        /**
         * @brief Reads the entire contents of @p path as a string.
         * @return The file contents, or an empty string on failure.
         */
        virtual std::string read_string(const std::filesystem::path& path) const = 0;

        /**
         * @brief Writes @p str to @p path, if this provider supports mutation.
         */
        virtual void write_string(const std::filesystem::path& path, const std::string& str) const = 0;

        /**
         * @brief Reads the contents of @p path as bytes, starting at @p offset.
         * @return The read bytes, or an empty vector if @p offset is out of range or the read fails.
         */
        virtual std::vector<std::byte> read_bytes(const std::filesystem::path& path, std::size_t offset) const = 0;

        /**
         * @brief Writes @p size bytes from @p data to @p path, if this provider supports mutation.
         */
        virtual void write_bytes(const std::filesystem::path& path, const void* data, std::size_t size) const = 0;

        /**
         * @brief Lists the entries directly inside the directory at @p path.
         * @return Paths of each entry, or an empty vector if @p path is not a directory.
         */
        virtual std::vector<std::filesystem::path> list(const std::filesystem::path& path) const = 0;

        /**
         * @brief Gets the most specific "absolute" representation of @p path this provider can offer.
         * Providers with no real filesystem backing (e.g. embedded resources) may just return @p path.
         */
        virtual std::string get_absolute_path(const std::filesystem::path& path) const = 0;

        /**
         * @brief Convenience factory that binds @p path to this provider.
         * Equivalent to constructing a FileHandle directly, but reads better at call sites
         * that already hold a concrete provider (e.g. `myProvider.open("assets/foo.png")`).
         */
        FileHandle open(const std::filesystem::path& path) const;
    };
}
