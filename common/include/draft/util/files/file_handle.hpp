#pragma once

#include "draft/util/files/file_provider.hpp"
#include "draft/util/time.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace Draft {
    /**
     * @brief A handle to a file or directory, backed by an injected FileProvider.
     *
     * FileHandle itself has no notion of *where* a file lives. Every operation is
     * delegated to whichever FileProvider it was constructed with.
     *
     * FileHandle owns a clone of its provider, so it has ordinary value semantics:
     * it can be freely copied, stored in containers, or used as a static/member
     * initializer without worrying about the provider's lifetime.
     */
    class FileHandle {
    public:
        /**
         * @brief Constructs a FileHandle for @p path, backed by a clone of @p provider.
         * @param path Path relative to the provider's root. Must not be empty.
         * @param provider The provider responsible for servicing operations on this handle.
         *                 Only its clone() is retained, so @p provider need not outlive this call.
         * @throws std::invalid_argument if @p path is empty.
         */
        FileHandle(const std::filesystem::path& path, const FileProvider& provider);

        FileHandle(const FileHandle& other);
        FileHandle(FileHandle&& other) noexcept = default;
        FileHandle& operator=(const FileHandle& other);
        FileHandle& operator=(FileHandle&& other) noexcept = default;
        ~FileHandle() = default;

        // Path information
        /**
         * @brief Checks if the file or directory exists.
         */
        bool exists() const;

        /**
         * @brief Checks if the handle points to a directory.
         */
        bool is_directory() const;

        /**
         * @brief Checks if the handle points to a regular file.
         */
        bool is_file() const;

        /**
         * @brief Gets the size of the file in bytes.
         */
        std::uintmax_t size() const;

        /**
         * @brief Gets the file extension (e.g., ".png").
         */
        std::string extension() const;

        /**
         * @brief Gets the file stem (filename without extension).
         */
        std::string stem() const;

        /**
         * @brief Gets the filename (with extension).
         */
        std::string filename() const;

        /**
         * @brief Gets the parent directory's FileHandle.
         */
        FileHandle parent() const;

        /**
         * @brief Gets the relative path as a string.
         */
        std::string get_path() const;

        /**
         * @brief Gets the provider's most specific "absolute" representation of this path.
         */
        std::string get_absolute_path() const;

        /**
         * @brief Gets the last modification time of the file.
         */
        Time last_modified() const;

        /**
         * @brief Gets the provider backing this handle.
         */
        const FileProvider& get_provider() const;

        // File Operations
        /**
         * @brief Removes the file or directory.
         */
        bool remove();

        /**
         * @brief Creates the parent directories for this file.
         */
        bool create_directories() const;

        /**
         * @brief Reads the entire content of the file as a string.
         */
        std::string read_string() const;

        /**
         * @brief Writes a string to the file.
         */
        void write_string(const std::string& str) const;

        /**
         * @brief Reads the content of the file as a vector of bytes.
         *
         * @param offset Byte offset to start reading from.
         * @return std::vector<std::byte> The read bytes.
         */
        std::vector<std::byte> read_bytes(std::size_t offset = 0) const;

        /**
         * @brief Writes a vector of bytes to the file.
         */
        void write_bytes(const std::vector<std::byte>& bytes) const;

        /**
         * @brief Writes raw data to the file.
         */
        void write_bytes(const void* data, std::size_t size) const;

        /**
         * @brief Lists all files and directories in this directory.
         */
        std::vector<FileHandle> list() const;

        // Operators
        /**
         * @brief Path join operator (joins paths with a separator).
         */
        FileHandle operator/(const std::filesystem::path& other) const;
        FileHandle& operator/=(const std::filesystem::path& other);

        /**
         * @brief String concatenation operator.
         */
        FileHandle operator+(const std::string& other) const;
        FileHandle& operator+=(const std::string& other);

        /**
         * @brief Two handles are equal if they share a path and the same kind of provider.
         */
        bool operator==(const FileHandle& other) const;
        bool operator!=(const FileHandle& other) const;

        friend std::ostream& operator<<(std::ostream& stream, const FileHandle& v) {
            stream << v.m_path.string();
            return stream;
        }

    private:
        std::filesystem::path m_path;
        std::unique_ptr<FileProvider> m_provider;
    };
}
