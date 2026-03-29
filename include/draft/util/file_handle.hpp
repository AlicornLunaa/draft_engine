#pragma once

#include "draft/util/time.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <ostream>

namespace Draft {
    /**
     * @brief A handle to a file or directory, which can be either local (filesystem) or internal (embedded resource).
     * FileHandle provides a unified interface for file operations, abstracting away whether the file is
     * physically on the disk or compiled into the engine.
     */
    class FileHandle {
    public:
        // Types
        enum Access { 
            INTERNAL, /** Embedded resource. */
            LOCAL    /** Standard filesystem file. */
        };

        // Constructors
        /**
         * @brief Default constructor. Creates an invalid FileHandle.
         */
        FileHandle();

        /**
         * @brief Constructs a FileHandle from a path and access type.
         */
        FileHandle(const std::filesystem::path& path, Access access = LOCAL);

        /**
         * @brief Constructs a FileHandle from a path string and access type.
         */
        FileHandle(const std::string& path, Access access = LOCAL);

        /**
         * @brief Constructs a FileHandle from a C-style path string and access type.
         */
        FileHandle(const char* path, Access access = LOCAL);

        /**
         * @brief Creates a local FileHandle.
         */
        static FileHandle local(const std::filesystem::path& path);

        /**
         * @brief Creates an internal FileHandle.
         */
        static FileHandle internal(const std::filesystem::path& path);

        /**
         * @brief Automatically detects whether to use LOCAL or INTERNAL access based on file existence.
         * Prefers LOCAL if the path exists on the filesystem.
         */
        static FileHandle automatic(const std::string& path);

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
         * @brief Gets the absolute path as a string (returns relative path for INTERNAL).
         */
        std::string get_absolute_path() const;

        /**
         * @brief Gets the last modification time of the file.
         */
        Time last_modified() const;

        /**
         * @brief Gets the access type (LOCAL or INTERNAL).
         */
        Access get_access() const { return m_access; }

        /**
         * @brief Checks if the FileHandle is valid (path is not empty).
         */
        bool is_valid() const;

        // File Operations
        /**
         * @brief Removes the file or directory (LOCAL only).
         */
        bool remove();

        /**
         * @brief Creates the parent directories for this file (LOCAL only).
         */
        bool create_directories() const;

        /**
         * @brief Reads the entire content of the file as a string.
         */
        std::string read_string() const;

        /**
         * @brief Writes a string to the file (LOCAL only).
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
         * @brief Writes a vector of bytes to the file (LOCAL only).
         */
        void write_bytes(const std::vector<std::byte>& bytes) const;

        /**
         * @brief Writes raw data to the file (LOCAL only).
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

        bool operator==(const FileHandle& other) const;
        bool operator!=(const FileHandle& other) const;

        /**
         * @brief Returns true if the FileHandle is valid.
         */
        explicit operator bool() const { return is_valid(); }

        friend std::ostream& operator<<(std::ostream& stream, const FileHandle& v) {
            stream << v.m_path.string();
            return stream;
        }

        // Legacy getters for backward compatibility
        std::string get_parent_path() const { return parent().get_path(); }
        long length() const { return static_cast<long>(size()); }

    private:
        std::filesystem::path m_path;
        Access m_access;
    };
}
