#include "draft/rendering/vertex_buffer.hpp"
#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"
#include "glad/gl.h"
#include <memory>

namespace Draft {
    // Inner class implementation
    template<typename T>
    VertexBuffer::Buffer<T>::Buffer(const std::vector<T>& data, int type){
        glGenBuffers(1, &vbo);
        glBindBuffer(type, vbo);
        glBufferData(type, data.size() * sizeof(T), &data[0], GL_STATIC_DRAW);
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
    void VertexBuffer::buffer(unsigned int index, const std::vector<int>& data, int type){
        bind();
        Buffer buffer(data, type); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }
    
    void VertexBuffer::buffer(unsigned int index, const std::vector<float>& data, int type){
        bind();
        Buffer buffer(data, type); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector2f>& data, int type){
        bind();
        Buffer buffer(data, type); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector3f>& data, int type){
        bind();
        Buffer buffer(data, type); // RAII implementation for glGenBuffers
        glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::start_buffer(const std::vector<float>& data, int type){
        bind();
        tempBuffer = std::make_unique<Buffer<float>>(data, type);
    }

    void VertexBuffer::set_attribute(unsigned int index, unsigned long count, unsigned long stride, unsigned long offset){
        glVertexAttribPointer(index, count, GL_FLOAT, GL_FALSE, stride, (void*)offset);
        glEnableVertexAttribArray(index);
    }

    void VertexBuffer::end_buffer(){
        unbind();
        tempBuffer.reset();
    }

    void VertexBuffer::bind(){
        glBindVertexArray(vao);
    }

    void VertexBuffer::unbind(){
        glBindVertexArray(0);
    }
};