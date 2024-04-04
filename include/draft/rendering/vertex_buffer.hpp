#pragma once
#define GL_ARRAY_BUFFER 0x8892

#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"

#include <vector>

namespace Draft {
    class VertexBuffer {
    private:
        // Inner class
        template<typename T>
        struct Buffer {
        private:
            unsigned int vbo;

        public:
            Buffer(const std::vector<T>& data, int type);
            ~Buffer();

            inline unsigned int get_vbo(){ return vbo; }
        };

        // Variables
        unsigned int vao;

    public:
        // Constructors
        VertexBuffer();
        ~VertexBuffer();

        // Functions
        void buffer(unsigned int index, const std::vector<int>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<float>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<Vector2f>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<Vector3f>& data, int type = GL_ARRAY_BUFFER);

        void bind();
        void unbind();
    };
};