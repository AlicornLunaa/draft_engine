#include "draft/rendering/image.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "stb_image_write.h"
#include "stb_image.h"
#include "glm/common.hpp"
#include <vector>

namespace Draft {
    // Private functions
    void Image::copy_data(const Image& other){
        // Copy data from other image provided
        if(data)
            delete[] data;

        size = other.size;
        colorSpace = other.colorSpace;
        pixelCount = other.pixelCount;
        data = new std::byte[other.pixelCount];

        // Deep copy raw data
        for(size_t i = 0; i < pixelCount; i++){
            data[i] = other.data[i];
        }
    }

    // Converters
    std::byte float_to_byte(float value){
        // Convert the 0-1 to 0-255
        float clamped = Math::clamp(value, 0.f, 1.f);
        char intermediate = (clamped * 255);
        std::byte byte = reinterpret_cast<std::byte&>(intermediate);
        return byte;
    }

    float byte_to_float(std::byte byte){
        // Convert the 0-1 to 0-255
        float val = reinterpret_cast<char&>(byte);
        return val / 255.f;
    };

    // Constructors
    Image::Image(unsigned int width, unsigned int height, Vector4f color, ColorSpace colorSpace) : size(width, height), colorSpace(colorSpace) {
        // Create data to hold
        pixelCount = width * height * static_cast<int>(colorSpace);
        data = new std::byte[pixelCount];

        // Fill the data, depending on the color space provided
        if(colorSpace == ColorSpace::GREYSCALE){
            // Greyscale
            std::byte avgColor = float_to_byte((color.r + color.g + color.b) / 3.f);

            for(size_t i = 0; i < pixelCount; i++){
                data[i] = avgColor;
            }
        } else if(colorSpace == ColorSpace::RGB){
            // RGB
            std::byte red = float_to_byte(color.r);
            std::byte green = float_to_byte(color.g);
            std::byte blue = float_to_byte(color.b);

            for(size_t i = 0; i < pixelCount; i += 3){
                data[i] = red;
                data[i + 1] = green;
                data[i + 2] = blue;
            }
        } else {
            // RGBA
            std::byte red = float_to_byte(color.r);
            std::byte green = float_to_byte(color.g);
            std::byte blue = float_to_byte(color.b);
            std::byte alpha = float_to_byte(color.a);

            for(size_t i = 0; i < pixelCount; i += 4){
                data[i] = red;
                data[i + 1] = green;
                data[i + 2] = blue;
                data[i + 3] = alpha;
            }
        }
    }

    Image::Image(unsigned int width, unsigned int height, ColorSpace colorSpace, const std::byte* pixelData) : size(width, height), colorSpace(colorSpace) {
        // Create data to hold this data, copy all available
        pixelCount = width * height * static_cast<int>(colorSpace);
        data = new std::byte[pixelCount];

        // Save data
        for(size_t i = 0; i < pixelCount; i++){
            data[i] = pixelData[i];
        }
    }

    Image::Image(const FileHandle& handle) : Image() { load(handle); }

    Image::Image(const Image& other) : size(other.size), colorSpace(other.colorSpace), pixelCount(other.pixelCount) {
        // Copy data
        copy_data(other);
    }

    Image::Image(Image&& other) noexcept {
        this->data = other.data;
        this->size = other.size;
        this->colorSpace = other.colorSpace;
        this->pixelCount = other.pixelCount;

        other.data = nullptr;
        other.size = {0, 0};
        other.colorSpace = ColorSpace::RGBA;
        other.pixelCount = 0;
    }

    Image::~Image(){
        if(data)
            // Null reference check, for sanity
            delete[] data;
    }

    // Operators
    Image& Image::operator=(const Image& other){
        if(this != &other){
            copy_data(other);
        }

        return *this;
    }

    Image& Image::operator=(Image&& other) noexcept {
        if(this != &other){
            this->data = other.data;
            this->size = other.size;
            this->colorSpace = other.colorSpace;
            this->pixelCount = other.pixelCount;

            other.data = nullptr;
            other.size = {0, 0};
            other.colorSpace = ColorSpace::RGBA;
            other.pixelCount = 0;
        }

        return *this;
    }

    // Functions
    void Image::load(const std::vector<std::byte>& arr){
        int width, height, channels;
        stbi_set_flip_vertically_on_load(false);
        unsigned char* pixelData = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(arr.data()), arr.size(), &width, &height, &channels, 0);

        if(data)
            delete[] data;

        size = { width, height };
        if(channels == 1) colorSpace = ColorSpace::GREYSCALE;
        else if(channels == 3) colorSpace = ColorSpace::RGB;
        else if(channels == 4) colorSpace = ColorSpace::RGBA;
        pixelCount = width * height * static_cast<int>(colorSpace);
        data = new std::byte[pixelCount];

        for(size_t i = 0; i < pixelCount; i++){
            unsigned char ch = pixelData[i];
            data[i] = reinterpret_cast<std::byte&>(ch);
        }

        stbi_image_free(pixelData);
    }

    void Image::load(const FileHandle& handle){
        load(handle.read_bytes());
    }

    void Image::save(FileHandle handle) const {
        std::vector<char> out(pixelCount);

        for(size_t i = 0; i < pixelCount; i++){
            std::byte byte = data[i];
            out[i] = reinterpret_cast<unsigned char&>(byte);
        }

        stbi_write_png(handle.get_path().c_str(), size.x, size.y, static_cast<int>(colorSpace), out.data(), 0);
    }

    void Image::mask(const Vector4f& color, float tolerance, std::byte alpha){
        // Conversion helper
        auto is_masked = [color, tolerance](const Vector4f& value){
            auto upperBound = color + color * tolerance;
            auto lowerBound = color - color * tolerance;
            return value.r >= lowerBound.r && value.g >= lowerBound.g && value.b >= lowerBound.b
                && value.r <= upperBound.r && value.g <= upperBound.g && value.b <= upperBound.b;
        };

        // Set alpha for everything not matching
        if(colorSpace == ColorSpace::GREYSCALE){
            // Greyscale
            for(size_t i = 0; i < pixelCount; i++){
                float value = byte_to_float(data[i]);

                if(!is_masked({ value, value, value, 1.f }))
                    data[i] = std::byte{0x0};
            }
        } else if(colorSpace == ColorSpace::RGB){
            // RGB
            for(size_t i = 0; i < pixelCount; i += 3){
                float red = byte_to_float(data[i]);
                float green = byte_to_float(data[i + 1]);
                float blue = byte_to_float(data[i + 2]);

                if(!is_masked({ red, green, blue, 1.f })){
                    data[i] = std::byte{0x0};
                    data[i + 1] = std::byte{0x0};
                    data[i + 2] = std::byte{0x0};
                }
            }
        } else {
            // RGBA
            for(size_t i = 0; i < pixelCount; i += 4){
                float red = byte_to_float(data[i]);
                float green = byte_to_float(data[i + 1]);
                float blue = byte_to_float(data[i + 2]);
                float alpha = byte_to_float(data[i + 3]);

                if(!is_masked({ red, green, blue, alpha }))
                    data[i + 3] = std::byte{0x0};
            }
        }
    }

    void Image::copy(const Image& src, Vector2u position, const IntRect& rect, bool applyAlpha){
        IntRect dimensions(rect);
        int stride = std::min(static_cast<int>(src.colorSpace), static_cast<int>(colorSpace));

        if(dimensions.width == 0 || dimensions.height == 0){
            dimensions.width = src.size.x;
            dimensions.height = src.size.y;
        }
        
        for(size_t x = dimensions.x; x < dimensions.width; x++){
            for(size_t y = dimensions.y; y < dimensions.height; y++){

                // Set each pixel frm the src to the destination
                for(int i = 0; i < stride; i++){
                    size_t srcIndex = x + y * src.size.x + i;
                    size_t targetIndex = (position.x + x) + (position.y + y) * size.x + i;

                    if(applyAlpha && src.colorSpace == ColorSpace::RGBA && i != 3){
                        float scalar = byte_to_float(src.data[x + y * src.size.x + 3]);
                        data[targetIndex] = float_to_byte(byte_to_float(src.data[srcIndex]) * scalar);
                    } else {
                        data[targetIndex] = src.data[srcIndex];
                    }
                }
            }
        }
    }

    void Image::flip_horizontally(){
        auto swap = [this](unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2){
            size_t index1 = x1 + y1 * size.x;
            size_t index2 = x2 + y2 * size.x;
            int stride = static_cast<int>(colorSpace);

            for(size_t i = 0; i < stride; i++){
                std::byte temp = data[index1 + i];
                data[index1 + i] = data[index2 + i];
                data[index2 + i] = temp;
            }
        };

        for(size_t y = 0; y < size.y; y++){
            for(size_t x = 0; x < size.x / 2; x++){
                swap(x, y, size.x - x, y);
            }
        }
    }

    void Image::flip_vertically(){
        auto swap = [this](unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2){
            size_t index1 = x1 + y1 * size.x;
            size_t index2 = x2 + y2 * size.x;
            int stride = static_cast<int>(colorSpace);

            for(size_t i = 0; i < stride; i++){
                std::byte temp = data[index1 + i];
                data[index1 + i] = data[index2 + i];
                data[index2 + i] = temp;
            }
        };

        for(size_t x = 0; x < size.x; x++){
            for(size_t y = 0; y < size.y / 2; y++){
                swap(x, y, x, size.y - y);
            }
        }
    }

    void Image::set_pixel(Vector2u position, Vector4f color){
        size_t startIndex = position.x + position.y * size.x;
        assert(startIndex < pixelCount && "Out of bounds");

        if(colorSpace == ColorSpace::GREYSCALE){
            data[startIndex] = float_to_byte((color.r + color.g + color.b) / 3.f * color.a);
        } else if(colorSpace == ColorSpace::RGB){
            data[startIndex] = float_to_byte(color.r);
            data[startIndex + 1] = float_to_byte(color.g);
            data[startIndex + 2] = float_to_byte(color.b);
        } else {
            data[startIndex] = float_to_byte(color.r);
            data[startIndex + 1] = float_to_byte(color.g);
            data[startIndex + 2] = float_to_byte(color.b);
            data[startIndex + 3] = float_to_byte(color.a);
        }
    }

    Vector4f Image::get_pixel(Vector2u position) const {
        size_t startIndex = position.x + position.y * size.x;
        assert(startIndex < pixelCount && "Out of bounds");

        if(colorSpace == ColorSpace::GREYSCALE){
            float val = byte_to_float(data[startIndex]);
            return {val, val, val, 1.f};
        } else if(colorSpace == ColorSpace::RGB){
            float red = byte_to_float(data[startIndex]);
            float green = byte_to_float(data[startIndex + 1]);
            float blue = byte_to_float(data[startIndex + 2]);
            return {red, green, blue, 1.f};
        } else {
            float red = byte_to_float(data[startIndex]);
            float green = byte_to_float(data[startIndex + 1]);
            float blue = byte_to_float(data[startIndex + 2]);
            float alpha = byte_to_float(data[startIndex + 3]);
            return {red, green, blue, alpha};
        }
    }
};