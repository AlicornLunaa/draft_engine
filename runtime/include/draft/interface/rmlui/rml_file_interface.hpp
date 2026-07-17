#pragma once

#include "draft/util/files/asset_file_system.hpp"

#include "RmlUi/Core/FileInterface.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace Draft {
    /**
     * @brief Routes RmlUi's own file I/O (documents loaded by path, @import'd style sheets,
     * font faces, non-PNG textures, ...) through Draft::AssetFileSystem, so content RmlUi
     * resolves on its own resolves loose-disk-then-embedded exactly like the rest of the engine.
     */
    class RmlFileInterface : public Rml::FileInterface {
    private:
        struct OpenFile {
            std::vector<std::byte> data;
            std::size_t cursor = 0;
        };

        AssetFileSystem m_fileSystem;
        std::unordered_map<Rml::FileHandle, OpenFile> m_openFiles;
        Rml::FileHandle m_nextHandle = 1;

    public:
        RmlFileInterface() = default;
        explicit RmlFileInterface(AssetFileSystem fileSystem);

        Rml::FileHandle Open(const Rml::String& path) override;
        void Close(Rml::FileHandle file) override;
        size_t Read(void* buffer, size_t size, Rml::FileHandle file) override;
        bool Seek(Rml::FileHandle file, long offset, int origin) override;
        size_t Tell(Rml::FileHandle file) override;
        size_t Length(Rml::FileHandle file) override;
    };
}
