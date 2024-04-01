#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "draft/rendering/shader.hpp"
#include "draft/util/logger.hpp"

using namespace std;

namespace Draft {
    // Pimpl definition
    struct Shader::Impl {

    };

    // Implementation
    Shader::Shader(const string& shaderPath) : ptr(make_unique<Impl>()) {
        // Build paths for files
        filesystem::path vertexPath(shaderPath);
        filesystem::path fragmentPath(shaderPath);
        vertexPath += "/vertex.spv";
        fragmentPath += "/fragment.spv";

        // Lambda to make loading shader simpler
        auto open_file = [](const filesystem::path& path){
            ifstream file(path, ios::ate | ios::binary);

            if(!file.is_open()){
                Logger::println(Level::SEVERE, "Shader", format("Unable to open file {}", path.c_str()));
                exit(0);
            }

            size_t fileSize = static_cast<size_t>(file.tellg());
            vector<char> buf(fileSize);

            file.seekg(0);
            file.read(buf.data(), fileSize);
            file.close();

            return buf;
        };

        // Load vertex shader
        auto vertexCode = open_file(vertexPath);
        auto fragmentCode = open_file(fragmentPath);

        cout << "Vertex size: " << vertexCode.size() << '\n';
        cout << "Fragment size: " << fragmentCode.size() << '\n';
    }

    Shader::~Shader(){
    }

    // Functions
    
};