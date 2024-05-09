#include "draft/util/file_handle.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <string>

#include "cmrc/cmrc.hpp"

CMRC_DECLARE(draft_engine);

namespace fs = std::filesystem;

namespace Draft {
    // Constructors
    FileHandle::FileHandle(const fs::path& path, Access access) : path(path), access(access) {}
    FileHandle::FileHandle(const char* path) : FileHandle(path, std::filesystem::exists(path) ? LOCAL : INTERNAL) {}
    FileHandle::FileHandle() : path("null"), access(INTERNAL) {}

    // Functions
    bool FileHandle::remove(){
        if(access == INTERNAL) return false;
        if(path == "null") return false;
        return fs::remove(path);
    }

    bool FileHandle::exists() const {
        if(path == "null") return false;

        switch(access){
        case LOCAL:
            return fs::exists(path);

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            return interalFiles.exists(path.string());
        }

        return false;
    }

    bool FileHandle::is_directory() const {
        if(access == INTERNAL) return false;
        if(path == "null") return false;
        return fs::is_directory(path);
    }

    long FileHandle::length() const {
        if(path == "null") return 0;

        switch(access){
        case LOCAL: {
            std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
            return in.tellg();
        }

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            const auto& data = interalFiles.open(path.string());
            return data.size();
        }

        return 0;
    }

    std::string FileHandle::filename() const {
        if(path == "null") return "";
        return path.filename().string();
    }

    std::string FileHandle::extension() const {
        if(path == "null") return "";
        fs::path p(path);
        return p.extension().string();
    }

    std::string FileHandle::get_path() const {
        if(path == "null") return "";
        return path.relative_path().string();
    }

    FileHandle::Access FileHandle::get_access() const {
        return access;
    }

    std::string FileHandle::read_string() const {
        if(path == "null") return "";

        std::string out{};

        switch(access){
            case LOCAL: {
                std::ifstream in(path);
                std::string line;

                while(std::getline(in, line)){
                    out += line + '\n';
                }
                
                return out;
            }

            case INTERNAL:
                const auto& interalFiles = cmrc::draft_engine::get_filesystem();
                const auto& data = interalFiles.open(path.string());

                for(auto iter = data.begin(); iter != data.end(); iter++){
                    out += *iter;
                }

                return out;
        }

        return "";
    }

    void FileHandle::write_string(const std::string& str){
        if(access == INTERNAL) return;
        if(path == "null") return;
        
        std::ofstream out(path);
        out << str;
    }

    std::vector<std::byte> FileHandle::read_bytes(long offset) const {
        if(path == "null") return {};

        std::vector<std::byte> out;
        std::streampos len;
        char* array;

        switch(access){
            case LOCAL: {
                std::ifstream in(path, std::ios::binary);

                len = in.tellg();
                in.seekg(0, std::ios::end);
                len = in.tellg() - len;
                array = new char[len];

                in.seekg(0, std::ios::beg);
                in.seekg(offset, std::ios::beg);
                in.read(array, len);
                break;
            }

            case INTERNAL:
                const auto& interalFiles = cmrc::draft_engine::get_filesystem();
                const auto& data = interalFiles.open(path.string());
                len = data.end() - data.begin();
                array = new char[len];

                int i = 0;

                for(auto iter = data.begin() + offset; iter != data.end(); iter++){
                    array[i] = *iter;
                    i++;
                }
                break;
        }

        out.resize(len);
        for(size_t i = 0; i < len; i++){
            out[i] = reinterpret_cast<std::byte&>(array[i]);
        }

        delete [] array;
        return out;
    }

    void FileHandle::write_bytes(const std::vector<std::byte>& array){
        if(access == INTERNAL) return;
        if(path == "null") return;
        
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(array.data()), array.size());
    }

    // Operators
    FileHandle FileHandle::operator+ (const std::string& right) const {
        std::string p = this->path.string();
        return { p + right, access };
    }

    FileHandle& FileHandle::operator+= (const std::string& right){
        path += right;
        return *this;
    }

    // Static functions
    FileHandle FileHandle::automatic(const std::string& path){
        // Find out of if there is an external file that exists, load it first
        // otherwise, load internal
        if(std::filesystem::exists(path)){
            // External file
            return FileHandle::local(path);
        }

        return FileHandle::internal(path);
    }
};