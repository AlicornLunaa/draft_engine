#pragma once

#include <unordered_map>
#include <string>
#include <variant>
#include <vector>

#include "draft/math/glm.hpp"
#include "draft/util/files/file_handle.hpp"

namespace Draft {
    /**
     * @brief Coarse type of a reflected vertex attribute or uniform (see Shader::reflect_attributes()/
     * reflect_uniforms()). Unknown covers anything not listed here (double vectors, sampler
     * types other than 2D, ...).
     */
    enum class ShaderDataType {
        Float, Vec2, Vec3, Vec4,
        Int, IVec2, IVec3, IVec4,
        UInt, UVec2, UVec3, UVec4,
        Bool,
        Mat2, Mat3, Mat4,
        Sampler2D,
        Unknown
    };

    /**
     * @brief One active `in` vertex attribute of a linked program.
     */
    struct ShaderAttribute {
        std::string name;
        int location;
        ShaderDataType type;
    };

    /**
     * @brief One active loose uniform of a linked program (uniform block members are not
     * reported, they have no single meaningful location to set through Shader::set_uniform()).
     */
    struct ShaderUniform {
        std::string name;
        int location;
        ShaderDataType type;
    };

    /**
     * @brief A compiled+linked vertex/fragment GL shader program. Do not construct before an
     * OpenGL context was established.
     */
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
        // Types
        using Uniform = std::variant<
            int,
            uint,
            float,
            double,
            bool,
            Vector2i,
            Vector3i,
            Vector4i,
            Vector2u,
            Vector3u,
            Vector4u,
            Vector2f,
            Vector3f,
            Vector4f,
            Vector2d,
            Vector3d,
            Vector4d,
            Matrix2,
            Matrix3,
            Matrix4
        >;

        // Constructors
        Shader(const FileHandle& vertexHandle, const FileHandle& fragmentHandle);
        Shader(const FileHandle& handle);
        Shader(const Shader& other) = delete;
        Shader(Shader&& other) noexcept;
        ~Shader();

        // Operators
        Shader& operator= (const Shader& other) = delete;

        // Functions
        inline unsigned int get_shader_handle() const { return shaderId; }
        void bind() const;
        void unbind() const;
        void reload();

        int get_location(const std::string& name) const;
        bool has_uniform(const std::string& name) const;

        /**
         * @brief Reflects every active (used by the linked program, not just declared) `in`
         * vertex attribute, via the GL_PROGRAM_INPUT program interface.
         */
        std::vector<ShaderAttribute> reflect_attributes() const;

        /**
         * @brief Reflects every active loose uniform (block members excluded), via the
         * GL_UNIFORM program interface.
         */
        std::vector<ShaderUniform> reflect_uniforms() const;

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
}
