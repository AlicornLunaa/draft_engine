#include <filesystem>
#include <string>

#include "draft/rendering/shader.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Private functions
    void Shader::cleanup(){
        glDeleteProgram(shaderId);
    }

    void Shader::load_shaders(const char* vertexSrc, const char* fragmentSrc){
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
            Logger::println(Level::SEVERE, "Shader", "Unable to compile vertex shader " + handle.filename() + " because\n" + infoLog);
            exit(0);
        }

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if(!success){
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            Logger::println(Level::SEVERE, "Shader", "Unable to compile fragment shader " + handle.filename() + " because\n" + infoLog);
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
            Logger::println(Level::SEVERE, "Shader", "Unable to link shader " + handle.filename() + " because\n" + infoLog);
            exit(0);
        }

        // Cleanup
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Shader::load_from_handle(const FileHandle& shaderHandle){
        // Build paths for files
        FileHandle vertexHandle = shaderHandle +  "/vertex.glsl";
        FileHandle fragmentHandle = shaderHandle +  "/fragment.glsl";
        
        auto vertexSrc = vertexHandle.read_bytes();
        auto fragmentSrc = fragmentHandle.read_bytes();
        vertexSrc.push_back('\0');
        fragmentSrc.push_back('\0');

        // Send data to OpenGL
        load_shaders(vertexSrc.data(), fragmentSrc.data());
    }

    // Constructors
    Shader::Shader(const char* vertexSrc, const char* fragmentSrc) : reloadable(false) {
        // Send data directly to OpenGL
        load_shaders(vertexSrc, fragmentSrc);
    }

    Shader::Shader(const FileHandle& handle) : reloadable(true), handle(handle) {
        // Build paths for files
        FileHandle vertexHandle = handle + "/vertex.glsl";
        FileHandle fragmentHandle = handle + "/fragment.glsl";
        
        auto vertexSrc = vertexHandle.read_bytes();
        auto fragmentSrc = fragmentHandle.read_bytes();
        vertexSrc.push_back('\0');
        fragmentSrc.push_back('\0');

        // Send data to OpenGL
        load_shaders(vertexSrc.data(), fragmentSrc.data());
    }

    Shader::Shader(const filesystem::path& shaderPath) : reloadable(true), handle(shaderPath, FileHandle::LOCAL) {
        load_from_handle(handle);
    }

    Shader::~Shader(){
        cleanup();
    }

    // Functions
    void Shader::bind() const {
        glUseProgram(shaderId);
    }

    void Shader::unbind() const {
        glUseProgram(0);
    }

    void Shader::reload(){
        if(!reloadable) return;
        unbind();
        cleanup();
        load_from_handle(handle);
    }

    int Shader::get_location(const std::string& name) const {
        // Check for memoized value first
        if(memo.find(name) != memo.end())
            return memo[name];

        int loc = glGetUniformLocation(shaderId, name.c_str());
        memo[name] = loc;

        if(loc == -1){
            Logger::println(Level::SEVERE, "Shader", "Uniform " + name + " does not exist on shader " + handle.filename());
        }

        return loc;
    }

    bool Shader::has_uniform(const std::string& name) const { return (glGetUniformLocation(shaderId, name.c_str()) != -1); }

    // Named uniforms
    void Shader::set_uniform(const std::string& name, bool value){ glUniform1i(get_location(name), value); }

    void Shader::set_uniform(const std::string& name, int value){ glUniform1i(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2i& value){ glUniform2i(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3i& value){ glUniform3i(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4i& value){ glUniform4i(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, unsigned int value){ glUniform1ui(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2u& value){ glUniform2ui(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3u& value){ glUniform3ui(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4u& value){ glUniform4ui(get_location(name), value.x, value.y, value.w, value.w); }

    void Shader::set_uniform(const std::string& name, float value){ glUniform1f(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2f& value){ glUniform2f(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3f& value){ glUniform3f(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4f& value){ glUniform4f(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, double value){ glUniform1d(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2d& value){ glUniform2d(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3d& value){ glUniform3d(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4d& value){ glUniform4d(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, const Matrix2& value){ glUniformMatrix2fv(get_location(name), 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(const std::string& name, const Matrix3& value){ glUniformMatrix3fv(get_location(name), 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(const std::string& name, const Matrix4& value){ glUniformMatrix4fv(get_location(name), 1, GL_FALSE, &value[0][0]); }

    // Location uniforms
    void Shader::set_uniform(int loc, bool value){ glUniform1i(loc, value); }

    void Shader::set_uniform(int loc, int value){ glUniform1i(loc, value); }
    void Shader::set_uniform(int loc, const Vector2i& value){ glUniform2i(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3i& value){ glUniform3i(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4i& value){ glUniform4i(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, unsigned int value){ glUniform1ui(loc, value); }
    void Shader::set_uniform(int loc, const Vector2u& value){ glUniform2ui(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3u& value){ glUniform3ui(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4u& value){ glUniform4ui(loc, value.x, value.y, value.w, value.w); }

    void Shader::set_uniform(int loc, float value){ glUniform1f(loc, value); }
    void Shader::set_uniform(int loc, const Vector2f& value){ glUniform2f(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3f& value){ glUniform3f(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4f& value){ glUniform4f(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, double value){ glUniform1d(loc, value); }
    void Shader::set_uniform(int loc, const Vector2d& value){ glUniform2d(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3d& value){ glUniform3d(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4d& value){ glUniform4d(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, const Matrix2& value){ glUniformMatrix2fv(loc, 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(int loc, const Matrix3& value){ glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(int loc, const Matrix4& value){ glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]); }
};