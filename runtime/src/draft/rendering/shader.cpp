#include "draft/rendering/shader.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

#include <stdexcept>
#include <vector>

namespace {
    Draft::ShaderDataType map_gl_type(GLenum type){
        switch(type){
            case GL_FLOAT: return Draft::ShaderDataType::Float;
            case GL_FLOAT_VEC2: return Draft::ShaderDataType::Vec2;
            case GL_FLOAT_VEC3: return Draft::ShaderDataType::Vec3;
            case GL_FLOAT_VEC4: return Draft::ShaderDataType::Vec4;
            case GL_INT: return Draft::ShaderDataType::Int;
            case GL_INT_VEC2: return Draft::ShaderDataType::IVec2;
            case GL_INT_VEC3: return Draft::ShaderDataType::IVec3;
            case GL_INT_VEC4: return Draft::ShaderDataType::IVec4;
            case GL_UNSIGNED_INT: return Draft::ShaderDataType::UInt;
            case GL_UNSIGNED_INT_VEC2: return Draft::ShaderDataType::UVec2;
            case GL_UNSIGNED_INT_VEC3: return Draft::ShaderDataType::UVec3;
            case GL_UNSIGNED_INT_VEC4: return Draft::ShaderDataType::UVec4;
            case GL_BOOL: return Draft::ShaderDataType::Bool;
            case GL_FLOAT_MAT2: return Draft::ShaderDataType::Mat2;
            case GL_FLOAT_MAT3: return Draft::ShaderDataType::Mat3;
            case GL_FLOAT_MAT4: return Draft::ShaderDataType::Mat4;
            case GL_SAMPLER_2D: return Draft::ShaderDataType::Sampler2D;
            default: return Draft::ShaderDataType::Unknown;
        }
    }
}

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
        glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
        glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
        glCompileShader(vertexShader);
        glCompileShader(fragmentShader);

        // Error check
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

        if(!success){
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::string message = "Unable to compile vertex shader " + handle.filename() + " because\n" + infoLog;
            Logger::println(LogLevel::Severe, "Shader", message);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            throw std::runtime_error(message);
        }

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

        if(!success){
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::string message = "Unable to compile fragment shader " + handle.filename() + " because\n" + infoLog;
            Logger::println(LogLevel::Severe, "Shader", message);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            throw std::runtime_error(message);
        }

        // Link to program
        shaderId = glCreateProgram();
        glAttachShader(shaderId, vertexShader);
        glAttachShader(shaderId, fragmentShader);
        glLinkProgram(shaderId);

        glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
        if(!success){
            glGetProgramInfoLog(shaderId, 512, nullptr, infoLog);
            std::string message = "Unable to link shader " + handle.filename() + " because\n" + infoLog;
            Logger::println(LogLevel::Severe, "Shader", message);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            glDeleteProgram(shaderId);
            shaderId = 0;
            throw std::runtime_error(message);
        }

        // Cleanup
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    void Shader::load_from_handle(const FileHandle& shaderHandle){
        // Build paths for files
        FileHandle vertexHandle = shaderHandle + "/vertex.glsl";
        FileHandle fragmentHandle = shaderHandle + "/fragment.glsl";

        auto vertexSrc = vertexHandle.read_bytes();
        auto fragmentSrc = fragmentHandle.read_bytes();
        vertexSrc.push_back(std::byte{'\0'});
        fragmentSrc.push_back(std::byte{'\0'});

        // Send data to OpenGL
        load_shaders(reinterpret_cast<const char*>(vertexSrc.data()), reinterpret_cast<const char*>(fragmentSrc.data()));
    }

    // Constructors
    Shader::Shader(const FileHandle& vertexHandle, const FileHandle& fragmentHandle) : reloadable(false), handle(vertexHandle){
        // Load shader data
        auto vertexSrc = vertexHandle.read_bytes();
        auto fragmentSrc = fragmentHandle.read_bytes();
        vertexSrc.push_back(std::byte{'\0'});
        fragmentSrc.push_back(std::byte{'\0'});

        // Send data to OpenGL
        load_shaders(reinterpret_cast<const char*>(vertexSrc.data()), reinterpret_cast<const char*>(fragmentSrc.data()));
    }

    Shader::Shader(const FileHandle& handle) : reloadable(true), handle(handle) {
        // Build paths for files
        FileHandle vertexHandle = handle + "/vertex.glsl";
        FileHandle fragmentHandle = handle + "/fragment.glsl";

        auto vertexSrc = vertexHandle.read_bytes();
        auto fragmentSrc = fragmentHandle.read_bytes();
        vertexSrc.push_back(std::byte{'\0'});
        fragmentSrc.push_back(std::byte{'\0'});

        // Send data to OpenGL
        load_shaders(reinterpret_cast<const char*>(vertexSrc.data()), reinterpret_cast<const char*>(fragmentSrc.data()));
    }

    Shader::Shader(Shader&& other) noexcept
        : shaderId(other.shaderId), reloadable(other.reloadable), handle(std::move(other.handle)), memo(std::move(other.memo))
    {
        // Stop the r-value from deleting the program when it's destroyed
        other.shaderId = 0;
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
        auto it = memo.find(name);
        if(it != memo.end())
            return it->second;

        int loc = glGetUniformLocation(shaderId, name.c_str());
        memo[name] = loc;

        if(loc == -1){
            Logger::println(LogLevel::Severe, "Shader", "Uniform " + name + " does not exist on shader " + handle.filename());
        }

        return loc;
    }

    bool Shader::has_uniform(const std::string& name) const { return (glGetUniformLocation(shaderId, name.c_str()) != -1); }

    std::vector<ShaderAttribute> Shader::reflect_attributes() const {
        std::vector<ShaderAttribute> attributes;

        int count = 0;
        glGetProgramInterfaceiv(shaderId, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &count);

        for(int i = 0; i < count; i++){
            const GLenum props[] = { GL_TYPE, GL_LOCATION };
            int values[2] = { 0, 0 };
            glGetProgramResourceiv(shaderId, GL_PROGRAM_INPUT, i, 2, props, 2, nullptr, values);

            if(values[1] < 0)
                continue; // Built-in input (gl_VertexID, ...), not a real attribute location

            char nameBuf[256];
            int nameLength = 0;
            glGetProgramResourceName(shaderId, GL_PROGRAM_INPUT, i, sizeof(nameBuf), &nameLength, nameBuf);

            attributes.push_back(ShaderAttribute{
                std::string(nameBuf, static_cast<size_t>(nameLength)),
                values[1],
                map_gl_type(static_cast<GLenum>(values[0]))
            });
        }

        return attributes;
    }

    std::vector<ShaderUniform> Shader::reflect_uniforms() const {
        std::vector<ShaderUniform> uniforms;

        int count = 0;
        glGetProgramInterfaceiv(shaderId, GL_UNIFORM, GL_ACTIVE_RESOURCES, &count);

        for(int i = 0; i < count; i++){
            const GLenum props[] = { GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };
            int values[3] = { 0, 0, 0 };
            glGetProgramResourceiv(shaderId, GL_UNIFORM, i, 3, props, 3, nullptr, values);

            if(values[2] != -1 || values[1] < 0)
                continue; // Uniform block member, or otherwise has no standalone location

            char nameBuf[256];
            int nameLength = 0;
            glGetProgramResourceName(shaderId, GL_UNIFORM, i, sizeof(nameBuf), &nameLength, nameBuf);

            uniforms.push_back(ShaderUniform{
                std::string(nameBuf, static_cast<size_t>(nameLength)),
                values[1],
                map_gl_type(static_cast<GLenum>(values[0]))
            });
        }

        return uniforms;
    }

    std::vector<ShaderStorageBlock> Shader::reflect_storage_blocks() const {
        std::vector<ShaderStorageBlock> blocks;

        int count = 0;
        glGetProgramInterfaceiv(shaderId, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &count);

        for(int i = 0; i < count; i++){
            const GLenum props[] = { GL_BUFFER_BINDING };
            int binding = 0;
            glGetProgramResourceiv(shaderId, GL_SHADER_STORAGE_BLOCK, i, 1, props, 1, nullptr, &binding);

            char nameBuf[256];
            int nameLength = 0;
            glGetProgramResourceName(shaderId, GL_SHADER_STORAGE_BLOCK, i, sizeof(nameBuf), &nameLength, nameBuf);

            blocks.push_back(ShaderStorageBlock{
                std::string(nameBuf, static_cast<size_t>(nameLength)),
                binding
            });
        }

        return blocks;
    }

    // Named uniforms
    void Shader::set_uniform(const std::string& name, bool value) const { glUniform1i(get_location(name), value); }

    void Shader::set_uniform(const std::string& name, int value) const { glUniform1i(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2i& value) const { glUniform2i(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3i& value) const { glUniform3i(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4i& value) const { glUniform4i(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, unsigned int value) const { glUniform1ui(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2u& value) const { glUniform2ui(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3u& value) const { glUniform3ui(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4u& value) const { glUniform4ui(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, float value) const { glUniform1f(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2f& value) const { glUniform2f(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3f& value) const { glUniform3f(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4f& value) const { glUniform4f(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, double value) const { glUniform1d(get_location(name), value); }
    void Shader::set_uniform(const std::string& name, const Vector2d& value) const { glUniform2d(get_location(name), value.x, value.y); }
    void Shader::set_uniform(const std::string& name, const Vector3d& value) const { glUniform3d(get_location(name), value.x, value.y, value.z); }
    void Shader::set_uniform(const std::string& name, const Vector4d& value) const { glUniform4d(get_location(name), value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(const std::string& name, const Matrix2& value) const { glUniformMatrix2fv(get_location(name), 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(const std::string& name, const Matrix3& value) const { glUniformMatrix3fv(get_location(name), 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(const std::string& name, const Matrix4& value) const { glUniformMatrix4fv(get_location(name), 1, GL_FALSE, &value[0][0]); }

    // Location uniforms
    void Shader::set_uniform(int loc, bool value) const { glUniform1i(loc, value); }

    void Shader::set_uniform(int loc, int value) const { glUniform1i(loc, value); }
    void Shader::set_uniform(int loc, const Vector2i& value) const { glUniform2i(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3i& value) const { glUniform3i(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4i& value) const { glUniform4i(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, unsigned int value) const { glUniform1ui(loc, value); }
    void Shader::set_uniform(int loc, const Vector2u& value) const { glUniform2ui(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3u& value) const { glUniform3ui(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4u& value) const { glUniform4ui(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, float value) const { glUniform1f(loc, value); }
    void Shader::set_uniform(int loc, const Vector2f& value) const { glUniform2f(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3f& value) const { glUniform3f(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4f& value) const { glUniform4f(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, double value) const { glUniform1d(loc, value); }
    void Shader::set_uniform(int loc, const Vector2d& value) const { glUniform2d(loc, value.x, value.y); }
    void Shader::set_uniform(int loc, const Vector3d& value) const { glUniform3d(loc, value.x, value.y, value.z); }
    void Shader::set_uniform(int loc, const Vector4d& value) const { glUniform4d(loc, value.x, value.y, value.z, value.w); }

    void Shader::set_uniform(int loc, const Matrix2& value) const { glUniformMatrix2fv(loc, 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(int loc, const Matrix3& value) const { glUniformMatrix3fv(loc, 1, GL_FALSE, &value[0][0]); }
    void Shader::set_uniform(int loc, const Matrix4& value) const { glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]); }
}
