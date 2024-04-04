#include <filesystem>
#include <format>
#include <fstream>

#include "draft/rendering/shader.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Private functions
    void Shader::load_shaders(const std::string& shaderPath){
        // Build paths for files
        filesystem::path vertexPath(shaderPath);
        filesystem::path fragmentPath(shaderPath);
        vertexPath += "/vertex.glsl";
        fragmentPath += "/fragment.glsl";

        // Lambda to make loading shader simpler
        auto open_file = [](const filesystem::path& path){
            ifstream file(path);
            string src{};
            string tmp{};

            if(!file.is_open()){
                Logger::println(Level::SEVERE, "Shader", format("Unable to open file {}", path.c_str()));
                exit(0);
            }

            while(getline(file, tmp)){
                src += tmp + '\n';
            }

            return src;
        };

        // Load shader sources
        string vertexCode = open_file(vertexPath);
        string fragmentCode = open_file(fragmentPath);
        auto vertexSrc = vertexCode.c_str();
        auto fragmentSrc = fragmentCode.c_str();

        // Allocate shaders
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Compile shader
        glShaderSource(vertexShader, 1, &vertexSrc, NULL);
        glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
        glCompileShader(vertexShader);
        glCompileShader(fragmentShader);

        // Error check
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if(!success){
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            Logger::println(Level::SEVERE, "Shader", format("Unable to compile vertex shader {} because\n{}", vertexPath.c_str(), infoLog));
            exit(0);
        }

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if(!success){
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            Logger::println(Level::SEVERE, "Shader", format("Unable to compile fragment shader {} because\n{}", fragmentPath.c_str(), infoLog));
            exit(0);
        }

        // Link to program
        shaderId = glCreateProgram();
        glAttachShader(shaderId, vertexShader);
        glAttachShader(shaderId, fragmentShader);
        glLinkProgram(shaderId);

        glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shaderId, 512, NULL, infoLog);
            Logger::println(Level::SEVERE, "Shader", format("Unable to link shader {} because\n{}", shaderPath, infoLog));
            exit(0);
        }

        // Cleanup
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // Constructors
    Shader::Shader(const string& shaderPath) : path(shaderPath) {
        load_shaders(shaderPath);
    }

    Shader::~Shader(){
        glDeleteProgram(shaderId);
    }

    // Functions
    void Shader::use(){
        glUseProgram(shaderId);
    }

    void Shader::set_uniform(){
        
    }
};