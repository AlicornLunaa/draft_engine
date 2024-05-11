#define STB_TRUETYPE_IMPLEMENTATION

#include <memory>

#include "draft/util/logger.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/font.hpp"

#include "glad/gl.h"
#include "stb_image_write.h"
#include "ft2build.h" // IWYU pragma: keep, Includes definitions for FT_FREETYPE_H
#include FT_FREETYPE_H

namespace Draft {
    // pImpl
    struct Font::Impl {
        FT_Library fontLibrary;
        FT_Face fontFace;
    };

    // Private functions
    void Font::clear(){
        for(auto* ptr : textures){
            delete ptr;
        }

        textures.clear();
        glyphs.clear();

        FT_Done_Face(ptr->fontFace);
        FT_Done_FreeType(ptr->fontLibrary);
    }

    void Font::load_font(unsigned int fontSize){
        // Initialize
        if(FT_Init_FreeType(&ptr->fontLibrary)){
            Logger::println(Level::CRITICAL, "Font", "Cannot initialize freetype");
            exit(0);
        }

        if(FT_New_Face(ptr->fontLibrary, handle.get_path().c_str(), 0, &ptr->fontFace)){
            Logger::println(Level::CRITICAL, "Font", "Cannot load font");
            exit(0);
        }

        // Load the textures
        FT_Set_Pixel_Sizes(ptr->fontFace, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Generate each glyph
        for(unsigned char c = 0; c < 128; c++){
            if(FT_Load_Char(ptr->fontFace, c, FT_LOAD_RENDER)){
                Logger::println(Level::CRITICAL, "Font", "Cannot load glyph" + std::to_string(c));
                continue;
            }

            Image image(ptr->fontFace->glyph->bitmap.width, ptr->fontFace->glyph->bitmap.rows, ColorSpace::GREYSCALE, reinterpret_cast<std::byte*>(ptr->fontFace->glyph->bitmap.buffer));
            image.flip_vertically();
            textures.push_back(new Texture(image, CLAMP_TO_EDGE));

            Glyph glyph{
                textures.back(),
                { ptr->fontFace->glyph->bitmap.width, ptr->fontFace->glyph->bitmap.rows },
                { ptr->fontFace->glyph->bitmap_left, ptr->fontFace->glyph->bitmap_top },
                ptr->fontFace->glyph->advance.x
            };

            glyphs.emplace(c, glyph);
        }
    }

    // Constructors
    Font::Font(const FileHandle& handle, unsigned int fontSize) : ptr(std::make_unique<Impl>()), handle(handle), fontSize(fontSize) {
        // Load font data
        load_font(fontSize);
    }

    Font::~Font(){
        clear();
    }

    // Functions
    const Font::Glyph& Font::get_glyph(char ch) const {
        // Check for glyph first
        auto iter = glyphs.find(ch);

        if(iter == glyphs.end()){
            Logger::println(Level::CRITICAL, "Font", "Glyph " + std::to_string(ch) + " is not loaded!");
            return glyphs.at(0);
        }

        // Otherwise return the glyph requested
        return glyphs.at(ch);
    }

    void Font::reload(){
        // Delete old textures
        clear();
        load_font(fontSize);
    }
};