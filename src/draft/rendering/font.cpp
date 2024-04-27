#include <memory>
#define STB_TRUETYPE_IMPLEMENTATION

#include "draft/util/logger.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/rendering/font.hpp"

#include "stb_image_write.h"
#include "ft2build.h" // IWYU pragma: keep, Includes definitions for FT_FREETYPE_H
#include FT_FREETYPE_H

namespace Draft {
    // pImpl
    struct Font::Impl {
        
    };

    // Constructors
    Font::Font(const FileHandle& handle) : ptr(std::make_unique<Impl>()) {
        // Load font data
        auto buffer = handle.read_bytes();
        auto bufferPtr = reinterpret_cast<const unsigned char*>(buffer.data());

        void* fontInfo;
        unsigned char* bitmap;
        int width, height;

        FT_Library library;
        FT_Face face;
        FT_GlyphSlot slot;

        if(FT_Init_FreeType(&library)){
            Logger::println(Level::CRITICAL, "Font", "Cannot initialize freetype.");
            exit(0);
        }

        FT_New_Face(library, handle.get_path().c_str(), 0, &face);
        FT_Set_Char_Size(face, 0, 20 * 64, 300, 300);
        slot = face->glyph;
        FT_Load_Char(face, 'a', FT_LOAD_RENDER);

        stbi_write_bmp("fuck.bmp", slot->bitmap.width, slot->bitmap.rows, 1, slot->bitmap.buffer);
    }

    Font::Font(const std::string& path) : Font(FileHandle{ path, FileHandle::LOCAL }) {}

    Font::~Font(){}
};