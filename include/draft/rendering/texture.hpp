#pragma once

#include <filesystem>

#include "draft/math/vector2.hpp"
#include "draft/util/file_handle.hpp"

namespace Draft {
    class Texture {
    public:
        // Enums
        enum Wrap { REPEAT = 0x2901, MIRRORED_REPEAT = 0x8370, CLAMP_TO_EDGE = 0x812F, CLAMP_TO_BORDER = 0x812D };
        enum Filter { NEAREST = 0x2600, LINEAR = 0x2601 };
        enum ColorSpace { RGB = 0x1907, RGBA = 0x1908 };
        
    private:
        // Variables
        const bool reloadable;
        bool loaded = false;

        FileHandle handle;
        Vector2i size;
        int nrChannels;

        unsigned int texId;

        // Private functions
        void generate_opengl(Wrap wrapping);
        void load_texture(const unsigned char* bytes, size_t length, bool flip = true);
        void cleanup();

    public:
        // Constructors
        Texture(Wrap wrapping = REPEAT);
        Texture(const unsigned char* start, const unsigned char* end, Wrap wrapping = REPEAT);
        Texture(const unsigned char* data, int width, int height, int channels, Wrap wrapping = REPEAT);
        Texture(const std::filesystem::path& texturePath, Wrap wrapping = REPEAT);
        Texture(const FileHandle& handle, Wrap wrapping = REPEAT);
        Texture(const Texture& other) = delete;
        ~Texture();
        
        // Functions
        inline bool is_loaded() const { return loaded; }
        inline const Vector2i& get_size() const { return size; }
        void bind(int unit = 0) const;
        void unbind() const;
        void reload();
    };
};