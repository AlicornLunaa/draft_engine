#pragma once
#define GL_ARRAY_BUFFER 0x8892

#include "draft/math/glm.hpp"

#include <cassert>
#include <vector>

namespace Draft {
    class VertexBuffer {
    public:
        // Enumerators
        enum Type { STATIC, DYNAMIC };

    private:
        // Inner class
        struct Buffer {
        private:
            unsigned int vbo;
            Type drawType;
            int arrayType;

            void gen_buffer(int type);

        public:
            template<typename T> Buffer(const std::vector<T>& data, int type); // Static buffer constructor
            Buffer(size_t bytes, int type); // Dynamic buffer constructor
            Buffer(const Buffer& other) = delete;
            ~Buffer();

            inline unsigned int get_vbo(){ return vbo; }
            inline Type get_draw_type(){ return drawType; }
            inline int get_array_type(){ return arrayType; }

            void bind();
            void unbind();
        };

        // Static vars
        static Buffer* tempBuffer;

        // Variables
        std::vector<Buffer*> buffers;
        unsigned int vao = 0;

        // Private functions
        void buffer_sub_data(int type, unsigned long offset, unsigned long size, const void* ptr);

    public:
        // Constructors
        VertexBuffer();
        VertexBuffer(const VertexBuffer& other) = delete;
        ~VertexBuffer();

        // Functions
        // These are short-hands for immediate buffer-to-attribute
        void buffer(unsigned int index, const std::vector<int>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<float>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<Vector2f>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<Vector3f>& data, int type = GL_ARRAY_BUFFER);
        void buffer(unsigned int index, const std::vector<Vector4f>& data, int type = GL_ARRAY_BUFFER);

        template<typename T>
        size_t start_buffer(size_t count, int type = GL_ARRAY_BUFFER){
            assert(!tempBuffer && "Buffer must be ended before starting another");
            bind();
            tempBuffer = new Buffer(count * sizeof(T), type);
            return buffers.size();
        }

        template<typename T>
        void set_dynamic_data(unsigned int index, const std::vector<T>& array, unsigned long offset = 0){
            // Sends data to the buffer
            auto* buf = buffers[index];
            assert(buf->get_draw_type() == DYNAMIC && "Buffer you're sending data to must be dynamic");
            buf->bind();
            buffer_sub_data(buf->get_array_type(), offset, array.size() * sizeof(T), array.data());
            buf->unbind();
        }

        void start_buffer(const std::vector<float>& data, int type = GL_ARRAY_BUFFER);
        void set_attribute(unsigned int index, int type, unsigned long count, unsigned long stride, unsigned long offset);
        void end_buffer();

        void bind() const;
        void unbind() const;
    };
};