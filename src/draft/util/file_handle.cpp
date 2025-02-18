#include "draft/util/file_handle.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <string>

#include "cmrc/cmrc.hpp"

CMRC_DECLARE(draft_engine);

namespace fs = std::filesystem;

namespace Draft {
    // Constructors
    FileHandle::FileHandle(const fs::path& path, Access access) : m_path(path), m_access(access) {}
    FileHandle::FileHandle(const char* path) : FileHandle(path, std::filesystem::exists(path) ? LOCAL : INTERNAL) {}
    FileHandle::FileHandle() : m_path("null"), m_access(INTERNAL) {}

    // Functions
    bool FileHandle::remove(){
        if(m_access == INTERNAL) return false;
        if(m_path == "null") return false;
        return fs::remove(m_path);
    }

    bool FileHandle::exists() const {
        if(m_path == "null") return false;

        switch(m_access){
        case LOCAL:
            return fs::exists(m_path);

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            return interalFiles.exists(m_path.string());
        }

        return false;
    }

    bool FileHandle::is_directory() const {
        if(m_access == INTERNAL) return false;
        if(m_path == "null") return false;
        return fs::is_directory(m_path);
    }

    long FileHandle::length() const {
        if(m_path == "null") return 0;

        switch(m_access){
        case LOCAL: {
            std::ifstream in(m_path, std::ifstream::ate | std::ifstream::binary);
            return in.tellg();
        }

        case INTERNAL:
            const auto& interalFiles = cmrc::draft_engine::get_filesystem();
            const auto& data = interalFiles.open(m_path.string());
            return data.size();
        }

        return 0;
    }

    std::string FileHandle::filename() const {
        if(m_path == "null") return "";
        return m_path.filename().string();
    }

    std::string FileHandle::extension() const {
        if(m_path == "null") return "";
        fs::path p(m_path);
        return p.extension().string();
    }

    std::string FileHandle::get_path() const {
        if(m_path == "null") return "";
        return m_path.relative_path().string();
    }

    Time FileHandle::last_modified() const {
        // Returns time the file was last modified
        if(m_path == "null") return Time::microseconds(0);
        auto lastModified = std::filesystem::last_write_time(m_path);
        unsigned long us = std::chrono::duration_cast<std::chrono::microseconds>(lastModified.time_since_epoch()).count();
        return Time::microseconds(us);
    }

    FileHandle::Access FileHandle::get_access() const {
        return m_access;
    }

    std::string FileHandle::read_string() const {
        if(m_path == "null") return "";

        std::string out{};

        switch(m_access){
            case LOCAL: {
                std::ifstream in(m_path);
                std::string line;

                while(std::getline(in, line)){
                    out += line + '\n';
                }
                
                return out;
            }

            case INTERNAL:
                const auto& interalFiles = cmrc::draft_engine::get_filesystem();
                const auto& data = interalFiles.open(m_path.string());

                for(auto iter = data.begin(); iter != data.end(); iter++){
                    out += *iter;
                }

                return out;
        }

        return "";
    }

    void FileHandle::write_string(const std::string& str){
        if(m_access == INTERNAL) return;
        if(m_path == "null") return;
        
        std::ofstream out(m_path);
        out << str;
    }

    std::vector<std::byte> FileHandle::read_bytes(long offset) const {
        if(m_path == "null") return {};

        std::vector<std::byte> out;
        std::streampos len;
        char* array;

        switch(m_access){
            case LOCAL: {
                std::ifstream in(m_path, std::ios::binary);

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
                const auto& data = interalFiles.open(m_path.string());
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
        if(m_access == INTERNAL) return;
        if(m_path == "null") return;
        
        std::ofstream out(m_path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(array.data()), array.size());
    }

    // Operators
    FileHandle FileHandle::operator+ (const std::string& right) const {
        std::string p = this->m_path.string();
        return { p + right, m_access };
    }

    FileHandle& FileHandle::operator+= (const std::string& right){
        m_path += right;
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