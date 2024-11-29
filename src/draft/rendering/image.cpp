#include "draft/rendering/image.hpp"
#include "draft/aliasing/format.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"

#include "stb_image_write.h"
#include "stb_image.h"

#include <string>
#include <vector>

namespace Draft {
    // Converters
    std::byte float_to_byte(float value){
        // Convert the 0-1 to 0-255
        float clamped = Math::clamp(value, 0.f, 1.f);
        char intermediate = (clamped * 255);
        std::byte byte = reinterpret_cast<std::byte&>(intermediate);
        return byte;
    }

    float byte_to_float(std::byte byte){
        // Convert the 0-255 to 1-0
        float val = reinterpret_cast<char&>(byte);
        return val / 255.f;
    };

    // Private functions
    void Image::copy_data(const Image& other){
        // Copy data from other image provided
        // If this was initialized before, remove the old data to prevent a memory leak. Dont bother removing the dangling pointer because it will be immediately reallocated.
        if(dataPtr)
            delete[] dataPtr;

        // Start copying the image-dependant data from the other image provided
        dataPtr = new std::byte[other.pixelCount];
        pixelCount = other.pixelCount;
        format = other.format;
        size = other.size;

        // Deep copy raw data
        for(size_t i = 0; i < pixelCount; i++){
            dataPtr[i] = other.dataPtr[i];
        }
    }

    // Constructors
    Image::Image(const Vector2u& size, Vector4f color, ColorFormat format) : size(size), format(format) {
        // Create the array to hold the image data. Pixel count is equal to the area of the image per channel.
        pixelCount = size.x * size.y * color_format_to_bytes(format);
        dataPtr = new std::byte[pixelCount];

        // Values to store into the image data. The color is in a float of 0-1 where the image needs a byte
        std::byte avgColor = float_to_byte((color.r + color.g + color.b) / 3.f); // Average here if being stored in greyscale
        std::byte byteValues[] = { float_to_byte(color.r), float_to_byte(color.g), float_to_byte(color.b), float_to_byte(color.a) };

        // Fill the image with the color data
        uint stride = color_format_to_bytes(format);

        for(size_t i = 0; i < pixelCount; i += stride){
            if(stride == 1){
                // Set the data to greyscale if there is only one color per pixel
                dataPtr[i] = avgColor;
            } else {
                // Otherwise set the data to RGBA respectively, cutting off when maxxing out the stride.
                // i.e. if there's only 3 channels, omit the alpha.
                for(uint k = 0; k < stride; k++){
                    dataPtr[i + k] = byteValues[k];
                }
            }
        }
    }

    Image::Image(const Vector2u& size, ColorFormat format, const std::byte* pixelData) : size(size), format(format) {
        // Create data to hold this data, copy all available
        pixelCount = size.x * size.y * color_format_to_bytes(format);
        dataPtr = new std::byte[pixelCount];

        // Save the data from the pointer into this new array
        for(size_t i = 0; i < pixelCount; i++){
            dataPtr[i] = pixelData[i];
        }
    }

    Image::Image(const std::vector<std::byte>& rawData) : Image() {
        load(rawData);
    }

    Image::Image(const FileHandle& handle) : Image() {
        load(handle);
    }

    Image::Image(const Image& other) {
        copy_data(other);
    }

    Image::Image(Image&& other) noexcept {
        // Move the data from other to here
        pixelCount = other.pixelCount;
        dataPtr = other.dataPtr;
        size = other.size; 
        format = other.format;

        // Zero out the previous image
        other.pixelCount = 0;
        other.dataPtr = nullptr;
        other.size = {0, 0};
        other.format = ColorFormat::RGBA;
    }

    Image::~Image(){
        // Check for nullptr, in case of the being moved
        if(dataPtr)
            delete[] dataPtr;
    }

    // Operators
    Image& Image::operator=(const Image& other){
        // Copy operator
        if(this != &other){
            copy_data(other);
        }

        return *this;
    }

    Image& Image::operator=(Image&& other) noexcept {
        // Move operator
        if(this != &other){
            // Delete old data if it exists
            if(dataPtr)
                delete[] dataPtr;

            // Save the other data from the other image
            pixelCount = other.pixelCount;
            dataPtr = other.dataPtr;
            size = other.size;
            format = other.format;

            // Zero out its data to prevent two things being the same
            other.pixelCount = 0;
            other.dataPtr = nullptr;
            other.size = {0, 0};
            other.format = ColorFormat::RGBA;
        }

        return *this;
    }

    // State functions
    void Image::load(const std::vector<std::byte>& arr){
        // Load the data from a byte array of PNG, JPEG, or similar
        stbi_set_flip_vertically_on_load(false);

        int width, height, channels;
        unsigned char* pixelData = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(arr.data()), arr.size(), &width, &height, &channels, 0);

        // Remove the old data pointer and its assosciated memory if it exists
        if(dataPtr)
            delete[] dataPtr;

        // Save new parameters from loaded image
        switch(channels){
        case 1:
            format = GREYSCALE;
            break;

        case 2:
            format = RG;
            break;

        case 3:
            format = RGB;
            break;

        case 4:
            format = RGBA;
            break;

        default:
            assert(false && "Unhandled channel count when loading image");
        }

        size = { width, height };
        pixelCount = width * height * color_format_to_bytes(format);
        dataPtr = new std::byte[pixelCount];

        // Pixel data here must be copied into the new array
        for(size_t i = 0; i < pixelCount; i++){
            unsigned char ch = pixelData[i];
            dataPtr[i] = reinterpret_cast<std::byte&>(ch);
        }

        // Remove the old pixel data to prevent memory leaks
        stbi_image_free(pixelData);
    }

    void Image::load(const FileHandle& handle){
        load(handle.read_bytes());
    }

    void Image::save(FileHandle handle) const {
        // Save this image to a text file. STB takes chars instead of bytes so cast it. and save it.
        std::vector<char> out(pixelCount);

        for(size_t i = 0; i < pixelCount; i++)
            out[i] = reinterpret_cast<unsigned char&>(dataPtr[i]);

        stbi_write_png(handle.get_path().c_str(), size.x, size.y, color_format_to_bytes(format), out.data(), 0);
    }

    // Manipulation functions
    void Image::mask(const Vector4f& color, float tolerance, std::byte alpha){
        // This lambda checks if the area is actually masked by the given color.
        auto is_masked = [color, tolerance](const Vector4f& value){
            auto upperBound = color + color * tolerance;
            auto lowerBound = color - color * tolerance;
            return value.r >= lowerBound.r && value.g >= lowerBound.g && value.b >= lowerBound.b
                && value.r <= upperBound.r && value.g <= upperBound.g && value.b <= upperBound.b;
        };

        // Run the mask algorithm over the bytes
        uint stride = color_format_to_bytes(format);

        for(size_t i = 0; i < pixelCount; i += stride){
            // Get the value of the byte and check if it is masked in order to zero out the data if its not
            bool masked = false;

            if(format == GREYSCALE){
                // Only check one channel, so do it as a greyscale value
                float value = byte_to_float(dataPtr[i]);
                masked = is_masked({ value, value, value, 1.f});
            } else {
                // Otherwise do RGBA, omitting extras as white
                float pixelValues[]{1, 1, 1, 1};

                for(uint k = 0; k < stride; k++){
                    pixelValues[k] = byte_to_float(dataPtr[i + k]);
                }

                masked = is_masked({ pixelValues[0], pixelValues[1], pixelValues[2], pixelValues[3] });
            }

            // Zero out alpha, if it has alpha, if not masked. If theres no alpha then zero out all color.
            if(!masked){
                if(format == RGBA){
                    dataPtr[i + 3] = std::byte{0x0};
                } else {
                    for(uint k = 0; k < stride; k++){
                        dataPtr[i + k] = std::byte{0x0};
                    }
                }
            }
        }
    }

    void Image::copy(const Image& src, const Vector2i& position, IntRect rect, bool applyAlpha){
        // Copies the source image to this image at position
        uint srcStride = color_format_to_bytes(src.format);
        uint destStride = color_format_to_bytes(format);

        // Check if the provided width or height is zero, if not auto-select the whole image. This is for convenience and nothing else
        if(rect.width <= 0) rect.width = src.size.x;
        if(rect.height <= 0) rect.height = src.size.y;

        Vector2u start{rect.x, rect.y};
        Vector2u end{rect.x + rect.width, rect.y + rect.height};

        // Run the algorithm for copying from the src to this data
        for(int y = start.y; y < end.y; y++){
            for(int x = start.x; x < end.x; x++){
                // Skip this pixel if its not on the actual image
                if(x + position.x >= size.x || x + position.x < 0 || y + position.y >= size.y || y + position.y < 0)
                    continue;

                // Set each pixel from the src to the destination. Depth stride is used here because its where the data is going to go anyways
                for(uint i = 0; i < destStride; i++){
                    // Obtain the indices for each the source and the target.
                    size_t srcIndex = (x + y * src.size.x) * srcStride + i;
                    size_t targetIndex = ((position.x + x) + (position.y + y) * size.x) * destStride + i;

                    // Make sure the index doesnt over-extend the bytes
                    if(i < srcStride){
                        // Final branch to check if it needs to apply alpha values to it or not. Useful for transparent copying
                        if(applyAlpha && src.format == RGBA && i != 3){
                            // Here it qualifies for it all, where it wants alpha, the source has alpha, and we're on the alpha component, meaning
                            // it should get the alpha value and apply it to the data.
                            float scalar = byte_to_float(src.dataPtr[x + y * src.size.x + 3]); // Alpha byte to float
                            dataPtr[targetIndex] = float_to_byte(byte_to_float(src.dataPtr[srcIndex]) * scalar);
                        } else {
                            // Here something has said to not have the alpha affect it, so instead just copy the data over like normal.
                            dataPtr[targetIndex] = src.dataPtr[srcIndex];
                        }
                    } else {
                        // White out the data, best I can do without more complex algorithms to do heuristics on the existing
                        // colors, which is useless for a small program like this.
                        dataPtr[targetIndex] = std::byte(0xFF);
                    }
                }
            }
        }
    }

    void Image::flip_horizontally(){
        // Swaps the pixel from left to right
        auto swap = [this](unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2){
            int stride = color_format_to_bytes(format);
            size_t index1 = (x1 + y1 * size.x) * stride;
            size_t index2 = (x2 + y2 * size.x) * stride;

            for(size_t i = 0; i < stride; i++){
                std::byte temp = dataPtr[index1 + i];
                dataPtr[index1 + i] = dataPtr[index2 + i];
                dataPtr[index2 + i] = temp;
            }
        };

        // Simple algorithm, no explanation needed
        for(size_t y = 0; y < size.y; y++){
            for(size_t x = 0; x < size.x / 2; x++){
                swap(x, y, size.x - x - 1, y);
            }
        }
    }

    void Image::flip_vertically(){
        // Swaps the pixel from top to bottom
        auto swap = [this](unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2){
            int stride = color_format_to_bytes(format);
            size_t index1 = (x1 + y1 * size.x) * stride;
            size_t index2 = (x2 + y2 * size.x) * stride;

            for(size_t i = 0; i < stride; i++){
                std::byte temp = dataPtr[index1 + i];
                dataPtr[index1 + i] = dataPtr[index2 + i];
                dataPtr[index2 + i] = temp;
            }
        };

        // Simple algorithm, no explanation needed
        for(size_t x = 0; x < size.x; x++){
            for(size_t y = 0; y < size.y / 2; y++){
                swap(x, y, x, size.y - y - 1);
            }
        }
    }

    void Image::set_pixel(const Vector2u& position, const Vector4f& color){
        // Sets the pixel to the image to the specified color
        uint stride = color_format_to_bytes(format);
        size_t index = (position.x + position.y * size.x) * stride;

        // If the index is out the pixel range, it cannot be within the texture
        assert(index < pixelCount && "Out of bounds");

        // Indexable color data
        float colorData[4]{color.r, color.g, color.b, color.a};

        // Set the data, if its greyscale use an average but if its not just omit any over extending
        if(format == GREYSCALE){
            // Avg data
            dataPtr[index] = float_to_byte((color.r + color.g + color.b) / 3.f * color.a);
        } else {
            // Save data
            for(uint k = 0; k < stride; k++){
                dataPtr[index + k] = float_to_byte(colorData[k]);
            }
        }
    }

    Vector4f Image::get_pixel(const Vector2u& position) const {
        // Gets the pixel's color data
        uint stride = color_format_to_bytes(format);
        size_t index = (position.x + position.y * size.x) * stride;

        // If the index is out the pixel range, it cannot be within the texture
        assert(index < pixelCount && "Out of bounds");

        // Indexable color data
        float colorData[4]{1.f, 1.f, 1.f, 1.f};

        // Get the data, if its greyscale use an average but if its not just omit any over extending
        if(format == ColorFormat::GREYSCALE){
            // Average all except alpha channel
            float val = byte_to_float(dataPtr[index]);
            colorData[0] = val;
            colorData[1] = val;
            colorData[2] = val;
        } else {
            // Iterate over the stride, anything thats not within will just be ignored
            for(uint k = 0; k < stride; k++){
                colorData[k] = byte_to_float(dataPtr[index + k]);
            }
        }

        return {colorData[0], colorData[1], colorData[2], colorData[3]};
    }
};