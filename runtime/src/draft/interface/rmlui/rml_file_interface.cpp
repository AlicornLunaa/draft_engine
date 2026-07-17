#include "draft/interface/rmlui/rml_file_interface.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace Draft {
    // Constructors
    RmlFileInterface::RmlFileInterface(AssetFileSystem fileSystem) : m_fileSystem(std::move(fileSystem)) {}

    // Functions
    Rml::FileHandle RmlFileInterface::Open(const Rml::String& path){
        if(!m_fileSystem.exists(path))
            return 0;

        OpenFile file;
        file.data = m_fileSystem.read_bytes(path);

        Rml::FileHandle handle = m_nextHandle++;
        m_openFiles.emplace(handle, std::move(file));
        return handle;
    }

    void RmlFileInterface::Close(Rml::FileHandle file){
        m_openFiles.erase(file);
    }

    size_t RmlFileInterface::Read(void* buffer, size_t size, Rml::FileHandle file){
        auto it = m_openFiles.find(file);
        if(it == m_openFiles.end())
            return 0;

        OpenFile& openFile = it->second;
        size_t remaining = openFile.data.size() - openFile.cursor;
        size_t toRead = std::min(size, remaining);

        std::memcpy(buffer, openFile.data.data() + openFile.cursor, toRead);
        openFile.cursor += toRead;
        return toRead;
    }

    bool RmlFileInterface::Seek(Rml::FileHandle file, long offset, int origin){
        auto it = m_openFiles.find(file);
        if(it == m_openFiles.end())
            return false;

        OpenFile& openFile = it->second;
        long base = 0;
        switch(origin){
            case SEEK_SET: base = 0; break;
            case SEEK_CUR: base = static_cast<long>(openFile.cursor); break;
            case SEEK_END: base = static_cast<long>(openFile.data.size()); break;
            default: return false;
        }

        long newPos = base + offset;
        if(newPos < 0 || static_cast<std::size_t>(newPos) > openFile.data.size())
            return false;

        openFile.cursor = static_cast<std::size_t>(newPos);
        return true;
    }

    size_t RmlFileInterface::Tell(Rml::FileHandle file){
        auto it = m_openFiles.find(file);
        return it == m_openFiles.end() ? 0 : it->second.cursor;
    }

    size_t RmlFileInterface::Length(Rml::FileHandle file){
        auto it = m_openFiles.find(file);
        return it == m_openFiles.end() ? 0 : it->second.data.size();
    }
}
