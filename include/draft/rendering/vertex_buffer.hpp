#pragma once

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
            Buffer(const std::vector<T>& data);
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
        void buffer(unsigned int index, const std::vector<int>& data);
        void buffer(unsigned int index, const std::vector<float>& data);
        void buffer(unsigned int index, const std::vector<Vector2f>& data);
        void buffer(unsigned int index, const std::vector<Vector3f>& data);

        void bind();
        void unbind();
    };
};