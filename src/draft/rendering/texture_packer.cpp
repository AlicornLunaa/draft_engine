#include "draft/rendering/texture_packer.hpp"
#include "draft/aliasing/format.hpp"
#include "draft/rendering/image.hpp"
#include <algorithm>
#include <memory>

namespace Draft {
    // Sorting struct
    struct PixelAreaSorter {
        inline bool operator() (const std::pair<const std::string&, const Image&>& pair1, const std::pair<const std::string&, const Image&>& pair2) {
            Vector2u size1 = pair1.second.get_size();
            Vector2u size2 = pair2.second.get_size();
            return (size1.x*size1.y > size2.x*size2.y);
        }
    };

    // Private functions
    void TexturePacker::double_size(){
        currentWidth *= 2;
        
        Image srcCopy(src);
        src = Image({currentWidth, currentWidth}, {0, 0, 0, 0});
        src.copy(srcCopy, {0, 0});
    }

    // Functions
    TextureRegion TexturePacker::get_region(const std::string& name) const {
        // Returns a new texture region with this rect
        auto iter = regions.find(name);

        if(iter != regions.end()){
            // Use this value
            return {*texture, iter->second};
        }

        // Failed to find, return an error region. error is the whole texture, just because tis easy to get
        return {*texture, {}};
    }

    void TexturePacker::clear(){
        src = Image({currentWidth, currentWidth}, {0, 0, 0, 0});
        regions.clear();
        texture.reset();
    }

    void TexturePacker::pack(std::vector<std::pair<std::string, Image>> data){
        // Add an image and pack it together to that its in the same texture data
        // Unsure of which algorithm to use but a post by zorba seems easy to implement (https://gamedev.stackexchange.com/questions/2829/texture-packing-algorithm)
        // Start off by sorting the elements in the array by size
        std::sort(data.begin(), data.end(), PixelAreaSorter());

        // Bail out early if nothing was provided
        if(data.empty())
            return;

        // If the initial texture is larger than the texture it has some weird stuff, could be fixed with math
        // but its a single edge case
        auto initialSize = data[0].second.get_size();

        while(std::max(initialSize.x, initialSize.y) >= currentWidth){
            double_size();
        }

        // Keep track of which image is currently in use
        Vector2u srcSize = src.get_size();
        Image mask(srcSize, {0, 0, 0, 0}, GREYSCALE);
        uint index = 0;

        // Start scanning the image
        for(Vector2u cursor = {}; cursor.y < srcSize.y - 1 && index != data.size(); cursor.x++){
            // Get the current image in order to determine if it can fit within remaining space
            auto& [key, img] = data[index];
            auto imgSize = img.get_size();

            // Wrap the cursor each scanline
            if(cursor.x >= srcSize.x - imgSize.x){
                cursor.x = 0;
                cursor.y++;
            }

            // If it extends past the bottom layer, its time to double the size
            if(cursor.y >= srcSize.y - imgSize.y){
                // Reset cursor, double the size
                double_size();
        
                Image maskCopy(mask);
                mask = Image(srcSize, {0, 0, 0, 0}, GREYSCALE);
                mask.copy(maskCopy, {0, 0});

                srcSize = src.get_size();
                cursor = {0, 0};
            }

            // Check if this spot is free for this entire section
            bool empty = true;
            for(uint y = cursor.y; y < cursor.y + imgSize.y && empty; y++){
                for(uint x = cursor.x; x < cursor.x + imgSize.x && empty; x++){
                    if(x >= srcSize.x || y >= srcSize.y)
                        continue;

                    if(mask.get_pixel({x, y}).r != 0){
                        empty = false;
                        break;
                    }
                }
            }

            // If its empty place it here and reset the cursor so it can work on the next image
            if(empty){
                // Add to the mask
                Image maskAddition(imgSize, {1, 1, 1, 1}, GREYSCALE);
                mask.copy(maskAddition, cursor);
                src.copy(img, cursor);

                // Save this key and rect so it may accessed later
                regions[key] = {(float)cursor.x, (float)cursor.y, (float)imgSize.x, (float)imgSize.y};

                cursor = {};
                index++;
            }
        }
    }

    void TexturePacker::pack(const std::vector<FileHandle>& handles){
        // Convert the list of filehandles to strings and images
        std::vector<std::pair<std::string, Image>> data;

        for(auto& handle : handles){
            data.emplace_back(handle.get_path(), Image(handle));
        }
        
        pack(data);
    }

    void TexturePacker::create(TextureProperties properties){
        // Create the OpenGL texture
        texture = std::make_unique<Texture>(src, properties);
    }
};