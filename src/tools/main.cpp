#include <fstream>
#include <iostream>
#include <string>
#include <getopt.h>

#include "draft/math/vector2.hpp"
#include "stb_image.h"

void print_help(){
    std::cout << "Engine Tools Help\n"
        << "\tSet input path to use: -i (path)\n"
        << "\tSet output path to use: -o (path)\n"
        << "\tCompile to static C++ data: -c\n"
        << "\tApplication help page: -h\n";
}

void compile_image(const std::string& input, const std::string& output){
    // Error check
    if(input.length() == 0){
        std::cout << "Provide an input path";
        exit(0);
    }

    if(output.length() == 0){
        std::cout << "Provide an output path";
        exit(0);
    }

    // Compile the image
    Draft::Vector2i size{};
    int nrChannels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(input.c_str(), &size.x, &size.y, &nrChannels, 0);
    size_t count = (size.x * size.y * nrChannels);

    if(!data){
        std::cout << "Error reading the image data\n";
        exit(0);
    }

    // Convert data to a C++ class
    std::string out = "#pragma once\n\n";
    out += "#include \"draft/core/compiled_asset_data.hpp\"\n\n";
    out += "static const StaticImageData IMAGE {" + std::to_string(size.x) + ", " + std::to_string(size.y) + ", " + std::to_string(nrChannels) + ", ";
    
    for(size_t i = 0; i < count; i++){
        out += std::to_string(data[i]) + ((i < count - 1) ? "," : "");
    }

    out += "};\n";

    // Save to file
    std::ofstream outFile(output);
    outFile << out;
    outFile.close();

    // Cleanup
    stbi_image_free(data);
}

int main(int argc, char** argv) {
    // Initial
    std::string inputFile{};
    std::string outputFile{};
    int opt;

    while((opt = getopt(argc, argv, "hi:o:c"))!= -1){
        switch(opt){
            case 'i':
                if(optarg == nullptr) exit(-1);
                inputFile = optarg;
                break;

            case 'o':
                if(optarg == nullptr) exit(-1);
                outputFile = optarg;
                break;
            
            case 'c':
                compile_image(inputFile, outputFile);
                break;
            
            case '?':
            case 'h':
            default:
                print_help();
                break;

            case -1:
                break;
        }
    }
}