#pragma once

#include <string>

#include "draft/math/matrix.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"

namespace Draft {
    class Shader {
    private:
        // Variables
        const std::string path;
        unsigned int shaderId;

        // Private variables
        void load_shaders(const std::string& shaderPath);
        unsigned int get_location(const std::string& name);

    public:
        // Constructors
        Shader(const std::string& shaderPath);
        Shader(const Shader& other) = delete;
        ~Shader();
        
        // Operators
        Shader& operator= (const Shader& other) = delete;

        // Functions
        void bind();
        void unbind();
        void reload();

        void set_uniform(const std::string& name, bool value);

        void set_uniform(const std::string& name, int value);
        void set_uniform(const std::string& name, const Vector2i& value);
        void set_uniform(const std::string& name, const Vector3i& value);

        void set_uniform(const std::string& name, unsigned int value);
        void set_uniform(const std::string& name, const Vector2u& value);
        void set_uniform(const std::string& name, const Vector3u& value);

        void set_uniform(const std::string& name, float value);
        void set_uniform(const std::string& name, const Vector2f& value);
        void set_uniform(const std::string& name, const Vector3f& value);

        void set_uniform(const std::string& name, double value);
        void set_uniform(const std::string& name, const Vector2d& value);
        void set_uniform(const std::string& name, const Vector3d& value);

        void set_uniform(const std::string& name, const Matrix2& value);
        void set_uniform(const std::string& name, const Matrix3& value);
        void set_uniform(const std::string& name, const Matrix4& value);
    };
};