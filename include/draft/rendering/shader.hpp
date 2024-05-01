#pragma once

#include <unordered_map>
#include <filesystem>
#include <string>

#include "draft/math/glm.hpp"
#include "draft/util/file_handle.hpp"

namespace Draft {
    class Shader {
    private:
        // Variables
        unsigned int shaderId;
        const bool reloadable;
        FileHandle handle;
        mutable std::unordered_map<std::string, int> memo;

        // Private variables
        void cleanup();
        void load_shaders(const char* vertexSrc, const char* fragmentSrc);
        void load_from_handle(const FileHandle& shaderHandle);

    public:
        // Constructors
        Shader(const char* vertexSrc, const char* fragmentSrc);
        Shader(const FileHandle& vertexHandle, const FileHandle& fragmentHandle);
        Shader(const FileHandle& handle);
        Shader(const std::filesystem::path& shaderPath);
        Shader(const Shader& other) = delete;
        ~Shader();
        
        // Operators
        Shader& operator= (const Shader& other) = delete;

        // Functions
        void bind() const;
        void unbind() const;
        void reload();

        int get_location(const std::string& name) const;
        bool has_uniform(const std::string& name) const;

        // Named uniforms
        void set_uniform(const std::string& name, bool value) const;

        void set_uniform(const std::string& name, int value) const;
        void set_uniform(const std::string& name, const Vector2i& value) const;
        void set_uniform(const std::string& name, const Vector3i& value) const;
        void set_uniform(const std::string& name, const Vector4i& value) const;

        void set_uniform(const std::string& name, unsigned int value) const;
        void set_uniform(const std::string& name, const Vector2u& value) const;
        void set_uniform(const std::string& name, const Vector3u& value) const;
        void set_uniform(const std::string& name, const Vector4u& value) const;

        void set_uniform(const std::string& name, float value) const;
        void set_uniform(const std::string& name, const Vector2f& value) const;
        void set_uniform(const std::string& name, const Vector3f& value) const;
        void set_uniform(const std::string& name, const Vector4f& value) const;

        void set_uniform(const std::string& name, double value) const;
        void set_uniform(const std::string& name, const Vector2d& value) const;
        void set_uniform(const std::string& name, const Vector3d& value) const;
        void set_uniform(const std::string& name, const Vector4d& value) const;

        void set_uniform(const std::string& name, const Matrix2& value) const;
        void set_uniform(const std::string& name, const Matrix3& value) const;
        void set_uniform(const std::string& name, const Matrix4& value) const;

        // By uniform location
        void set_uniform(int loc, bool value) const;

        void set_uniform(int loc, int value) const;
        void set_uniform(int loc, const Vector2i& value) const;
        void set_uniform(int loc, const Vector3i& value) const;
        void set_uniform(int loc, const Vector4i& value) const;

        void set_uniform(int loc, unsigned int value) const;
        void set_uniform(int loc, const Vector2u& value) const;
        void set_uniform(int loc, const Vector3u& value) const;
        void set_uniform(int loc, const Vector4u& value) const;

        void set_uniform(int loc, float value) const;
        void set_uniform(int loc, const Vector2f& value) const;
        void set_uniform(int loc, const Vector3f& value) const;
        void set_uniform(int loc, const Vector4f& value) const;

        void set_uniform(int loc, double value) const;
        void set_uniform(int loc, const Vector2d& value) const;
        void set_uniform(int loc, const Vector3d& value) const;
        void set_uniform(int loc, const Vector4d& value) const;

        void set_uniform(int loc, const Matrix2& value) const;
        void set_uniform(int loc, const Matrix3& value) const;
        void set_uniform(int loc, const Matrix4& value) const;

        template<typename T>
        void set_uniform_if_exists(const std::string& name, T value) const {
            if(has_uniform(name)){
                set_uniform(name, value);
            }
        }
    };
};