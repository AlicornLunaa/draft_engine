#include "draft/rendering/vertex_buffer.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"
#include "glad/gl.h"

namespace Draft {
    // Inner class implementation
    template<typename T>
    VertexBuffer::Buffer<T>::Buffer(const std::vector<T>& data){
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), &data[0], GL_STATIC_DRAW);
    }

    template<typename T>
    VertexBuffer::Buffer<T>::~Buffer(){
        glDeleteBuffers(1, &vbo);
    }

    template class VertexBuffer::Buffer<int>;
    template class VertexBuffer::Buffer<float>;
    template class VertexBuffer::Buffer<Vector2f>;
    template class VertexBuffer::Buffer<Vector3f>;

    // Constructors
    VertexBuffer::VertexBuffer(){
        glGenVertexArrays(1, &vao);
    }

    VertexBuffer::~VertexBuffer(){
        glDeleteVertexArrays(1, &vao);
    }

    // Functions
    void VertexBuffer::buffer(unsigned int index, const std::vector<int>& data){
        bind();
        Buffer buffer(data); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }
    
    void VertexBuffer::buffer(unsigned int index, const std::vector<float>& data){
        bind();
        Buffer buffer(data); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector2f>& data){
        bind();
        Buffer buffer(data); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector3f>& data){
        bind();
        Buffer buffer(data); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::bind(){
        glBindVertexArray(vao);
    }

    void VertexBuffer::unbind(){
        glBindVertexArray(0);
    }
};