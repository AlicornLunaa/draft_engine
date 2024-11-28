#pragma once

#include "draft/aliasing/format.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/util/file_handle.hpp"

#include <cstddef>
#include <vector>

namespace Draft {
    class Image {
    private:
        // Variables
        Vector2u size;
        ColorFormat colorSpace;
        std::byte* data = nullptr;
        size_t pixelCount = 0;

        // Private functions
        void copy_data(const Image& other);

    public:
        // Constructors
        Image(unsigned int width = 1, unsigned int height = 1, Vector4f color = {1, 1, 1, 1}, ColorFormat colorSpace = ColorFormat::RGBA);
        Image(unsigned int width, unsigned int height, ColorFormat colorSpace, const std::byte* pixelData);
        Image(const std::vector<std::byte>& rawData, bool flip = false);
        Image(const FileHandle& handle, bool flip = false);
        Image(const Image& other);
        Image(Image&& other) noexcept;
        ~Image();

        // Operators
        Image& operator=(const Image& other);
        Image& operator=(Image&& other) noexcept;

        // Functions
        void load(const std::vector<std::byte>& arr, float flip = false);
        void load(const FileHandle& handle, float flip = false);
        void save(FileHandle handle) const;
        void reload();

        void mask(const Vector4f& color, float tolerance = 0.f, std::byte alpha = std::byte{ 0x0 });
        void copy(const Image& src, Vector2u position, const IntRect& rect = {0, 0, 0, 0}, bool applyAlpha = false);
        void flip_horizontally();
        void flip_vertically();
        void set_pixel(Vector2u position, Vector4f color);
        Vector4f get_pixel(Vector2u position) const;

        inline const std::byte* c_arr() const { return data; }
        inline const Vector2u& get_size() const { return size; }
        inline ColorFormat get_color_space() const { return colorSpace; }
        inline size_t get_pixel_count() const { return pixelCount; }
        inline bool is_transparent() const { return colorSpace == ColorFormat::RGBA; }
    };
};