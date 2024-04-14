#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Draft {
    class FileHandle {
    public:
        // Enumerators
        enum Access { INTERNAL, LOCAL };
        
    private:
        // Variables
        std::filesystem::path path;
        Access access;

    public:
        // Constructors
        FileHandle(const std::string& path, Access access);

        // Functions
        bool remove();
        bool exists() const;
        bool is_directory() const;
        long length() const;
        std::string extension() const;
        std::string get_path() const;
        Access get_access() const;

        std::string read_string() const;
        void write_string(const std::string& str);

        std::vector<char> read_bytes(long offset = 0) const;
        void write_bytes(const char* array, long size);

        friend std::ostream& operator<< (std::ostream& stream, const FileHandle& v){
            stream << v.path;
            return stream;
        }

        // Static functions
        inline static FileHandle internal(const std::string& path){ return FileHandle(path, INTERNAL); }
        inline static FileHandle local(const std::string& path){ return FileHandle(path, LOCAL); }
    };
};