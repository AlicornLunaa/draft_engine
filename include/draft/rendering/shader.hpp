#pragma once

#include <memory>
#include <string>

namespace Draft {
    class Shader {
    public:
        // Constructors
        Shader(const std::string& shaderPath);
        Shader(const Shader& other) = delete;
        ~Shader();
        
        // Operators
        Shader& operator= (const Shader& other) = delete;

        // Functions

    private:
        // Pimpl
        struct Impl;
        std::unique_ptr<Impl> ptr;
    };
};