#include "draft/util/file_handle.hpp"

#include <fstream>
#include <string>

#include "cmrc/cmrc.hpp"

CMRC_DECLARE(draft_engine);

namespace fs = std::filesystem;

namespace Draft {
    // Constructors
    FileHandle::FileHandle(const std::string& path, Access access) : path(path), access(access) {}

    // Functions
    bool FileHandle::remove(){
        if(access == INTERNAL) return false;
        return fs::remove(path);
    }

    bool FileHandle::exists() const {
        switch(access){
        case LOCAL:
            return fs::exists(path);

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            return interalFiles.exists(path);
        }

        return false;
    }

    bool FileHandle::is_directory() const {
        if(access == INTERNAL) return false;
        return fs::is_directory(path);
    }

    long FileHandle::length() const {
        switch(access){
        case LOCAL: {
            std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
            return in.tellg();
        }

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            const auto& data = interalFiles.open(path);
            return data.size();
        }

        return 0;
    }

    std::string FileHandle::extension() const {
        fs::path p(path);
        return p.extension();
    }

    std::string FileHandle::get_path() const {
        return path.relative_path();
    }

    FileHandle::Access FileHandle::get_access() const {
        return access;
    }

    std::string FileHandle::read_string() const {
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
                const auto& data = interalFiles.open(path);

                for(auto iter = data.begin(); iter != data.end(); iter++){
                    out += *iter;
                }

                return out;
        }

        return "";
    }

    void FileHandle::write_string(const std::string& str){
        if(access == INTERNAL) return;
        
        std::ofstream out(path);
        out << str;
    }

    std::vector<char> FileHandle::read_bytes(long offset) const {
        std::vector<char> out{};
        char array[length()];

        switch(access){
            case LOCAL: {
                std::ifstream in(path);
                in.read(array, length());
                break;
            }

            case INTERNAL:
                const auto& interalFiles = cmrc::draft_engine::get_filesystem();
                const auto& data = interalFiles.open(path);
                int i = 0;

                for(auto iter = data.begin(); iter != data.end(); iter++){
                    array[i] = *iter;
                    i++;
                }
                break;
        }

        for(size_t i = 0; i < length(); i++){
            out.push_back(array[i]);
        }

        return out;
    }

    void FileHandle::write_bytes(const char* array, long size){
        if(access == INTERNAL) return;
        
        std::ofstream out(path);
        out.write(array, size);
    }
};