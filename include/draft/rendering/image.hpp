#pragma once

#include "draft/aliasing/format.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/util/file_handle.hpp"

#include <cstddef>
#include <vector>

namespace Draft {
    /**
     * @brief Image is similar to Texture, however the data lives on the processor and RAM. This allows
     *  for more control over the data. It has no connection to OpenGL which means it can be loaded before
     *  the OpenGL context is created.
     */
    class Image {
    private:
        // Variables
        std::byte* dataPtr = nullptr;
        size_t pixelCount = 0;

        ColorFormat format;
        Vector2u size;

        // Private functions
        void copy_data(const Image& other);

    public:
        // Constructors
        Image(const Vector2u& size = {1, 1}, Vector4f color = {1, 1, 1, 1}, ColorFormat format = ColorFormat::RGBA); // Image constructor from no data
        Image(const Vector2u& size, ColorFormat format, const std::byte* pixelDataArr); // Raw data constructor for raw pixel data
        Image(const std::vector<std::byte>& rawData); // Compressed data constructor, stuff like PNG or JPEG
        Image(const FileHandle& handle); // Load from file
        Image(const Image& other); // Copy constructor
        Image(Image&& other) noexcept; // Move constructor
        ~Image(); // Destructor

        // Operators
        Image& operator=(const Image& other);
        Image& operator=(Image&& other) noexcept;

        // State functions
        /**
         * @brief Loads from compressed texture data.
         * @param arr PNG, JPEG, etc
         */
        void load(const std::vector<std::byte>& arr);

        /**
         * @brief Load from a file
         * @param handle 
         */
        void load(const FileHandle& handle);

        /**
         * @brief Save to a file
         * @param handle 
         */
        void save(FileHandle handle) const;

        // Editing functions
        void mask(const Vector4f& color, float tolerance = 0.f, std::byte alpha = std::byte{ 0x00 });
        void copy(const Image& src, const Vector2i& position, IntRect rect = {0, 0, 0, 0}, bool applyAlpha = false);
        void flip_horizontally();
        void flip_vertically();
        void set_pixel(const Vector2u& position, const Vector4f& color);
        Vector4f get_pixel(const Vector2u& position) const;

        // Getters
        inline ColorFormat get_format() const { return format; }
        inline const Vector2u& get_size() const { return size; }
        inline size_t get_pixel_count() const { return pixelCount; }
        inline const std::byte* c_arr() const { return dataPtr; }
        inline bool is_transparent() const { return format == ColorFormat::RGBA; }
    };
};