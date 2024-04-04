#pragma once

#include <string>

namespace Draft {
    class Shader {
    private:
        // Variables
        const std::string path;
        unsigned int shaderId;

        // Private variables
        void load_shaders(const std::string& shaderPath);

    public:
        // Constructors
        Shader(const std::string& shaderPath);
        Shader(const Shader& other) = delete;
        ~Shader();
        
        // Operators
        Shader& operator= (const Shader& other) = delete;

        // Functions
        void use();
        void set_uniform();
    };
};