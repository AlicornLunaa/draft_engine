#define STB_TRUETYPE_IMPLEMENTATION

#include <memory>
#include <algorithm>

#include "draft/util/logger.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/font.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

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
    void Font::load_font(){
        // Initialize
        if(FT_Init_FreeType(&ptr->fontLibrary)){
            Logger::println(Level::CRITICAL, "Font", "Cannot initialize freetype");
            exit(0);
        }

        if(FT_New_Memory_Face(ptr->fontLibrary, reinterpret_cast<unsigned char*>(rawData.data()), rawData.size(), 0, &ptr->fontFace)){
            Logger::println(Level::CRITICAL, "Font", "Cannot load font");
            exit(0);
        }

        // Pack each glyph into the texture
        for(unsigned char c = 0; c < 128; c++)
            bake_glyph(c);
    }

    void Font::bake_glyph(char ch) const {
        // Check for the mapped value
        if(fontSizeToTextureMap.find(fontSize) == fontSizeToTextureMap.end()){
            // This size isnt baked yet, create a texture for this type
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            fontSizeToTextureMap[fontSize] = fontTypes.size();

            auto& ref = fontTypes.emplace_back(
                std::unordered_map<char, Glyph>{},
                std::vector<std::shared_ptr<Texture>>{},
                std::vector<Image>{},
                IntRect{0, 0, 0, 0},
                0,
                fontSize
            );

            ref.images.emplace_back(2048, 2048, Vector4f{0, 0, 0, 0}, ColorSpace::GREYSCALE);
            ref.textures.push_back(std::shared_ptr<Texture>(new Texture(ref.images.back(), CLAMP_TO_EDGE)));
        }

        // Get font type data
        auto& fontType = fontTypes[fontSizeToTextureMap[fontSize]];
        auto& baseImage = fontType.images.back();
        auto baseTexture = fontType.textures.back();

        // Load this character into the font glyph
        FT_Set_Pixel_Sizes(ptr->fontFace, 0, fontSize);
        if(FT_Load_Char(ptr->fontFace, ch, FT_LOAD_RENDER)){
            Logger::println(Level::CRITICAL, "Font", "Cannot load glyph" + std::to_string(ch));
            fontType.glyphs.emplace(ch, get_glyph(0));
            return;
        }

        // Calculates bounds to store the glyph in the texture
        IntRect bounds = {
            fontType.previousGlyphBounds.x + fontType.previousGlyphBounds.width,
            fontType.previousGlyphBounds.y,
            static_cast<int>(ptr->fontFace->glyph->bitmap.width),
            static_cast<int>(ptr->fontFace->glyph->bitmap.rows)
        };
        fontType.rowDepth = std::max(fontType.rowDepth, bounds.height);
        fontType.previousGlyphBounds = bounds;

        // Shift the bounds to the next line
        if(bounds.x + bounds.width >= baseImage.get_size().x){
            // Shift to next line
            bounds.x = 0;
            bounds.y += fontType.rowDepth;
            fontType.rowDepth = 0;
        }

        // Shift to a new texture
        if(bounds.y + bounds.height >= baseImage.get_size().y){
            fontType.previousGlyphBounds = {0, 0, 0, 0};
            fontType.rowDepth = 0;
            fontType.images.emplace_back(2048, 2048, Vector4f{0, 0, 0, 0}, ColorSpace::GREYSCALE);
            fontType.textures.push_back(std::shared_ptr<Texture>(new Texture(fontType.images.back(), CLAMP_TO_EDGE)));
            baseImage = fontType.images.back();
            baseTexture = fontType.textures.back();
        }

        // Copy the glyph data to the base image
        Image image(ptr->fontFace->glyph->bitmap.width, ptr->fontFace->glyph->bitmap.rows, ColorSpace::GREYSCALE, reinterpret_cast<std::byte*>(ptr->fontFace->glyph->bitmap.buffer));
        image.flip_vertically();
        baseImage.copy(image, {bounds.x, bounds.y});
        baseTexture->update(baseImage);

        // Save glyph data
        fontType.glyphs.emplace(ch, Glyph{
            TextureRegion{baseTexture, bounds},
            { ptr->fontFace->glyph->bitmap.width, ptr->fontFace->glyph->bitmap.rows },
            { ptr->fontFace->glyph->bitmap_left, ptr->fontFace->glyph->bitmap_top },
            ptr->fontFace->glyph->advance.x
        });
    }
    
    void Font::clear(){
        fontSizeToTextureMap.clear();
        fontTypes.clear();

        FT_Done_Face(ptr->fontFace);
        FT_Done_FreeType(ptr->fontLibrary);
    }

    // Constructors
    Font::Font(const FileHandle& handle) : ptr(std::make_unique<Impl>()), handle(handle) {
        // Load font data
        rawData = handle.read_bytes();
        load_font();
    }

    Font::Font(const std::vector<std::byte>& rawData) : ptr(std::make_unique<Impl>()) {
        this->rawData = rawData;
        load_font();
    }

    Font::~Font(){
        clear();
    }

    // Functions
    const Font::Glyph& Font::get_glyph(char ch) const {
        // Make sure fontType exists
        if(fontSizeToTextureMap.find(fontSize) == fontSizeToTextureMap.end()){
            bake_glyph(0);
        }

        // Get glyph in this font type
        auto& fontType = fontTypes[fontSizeToTextureMap.at(fontSize)];
        auto iter = fontType.glyphs.find(ch);

        if(iter == fontType.glyphs.end()){
            bake_glyph(ch);
            return fontType.glyphs.at(ch);
        }

        // Otherwise return the glyph requested
        return fontType.glyphs.at(ch);
    }

    void Font::reload(){
        // Delete old textures
        clear();
        load_font();
    }
};