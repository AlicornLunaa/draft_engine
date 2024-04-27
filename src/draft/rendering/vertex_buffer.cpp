#include "draft/rendering/vertex_buffer.hpp"
#include "draft/math/glm.hpp"
#include "glad/gl.h"
#include <cassert>

namespace Draft {
    // Base class for VAO
    // Inner class implementation
    void VertexBuffer::Buffer::gen_buffer(int type){
        glCreateBuffers(1, &vbo);
        bind();
    }

    template<typename T>
    VertexBuffer::Buffer::Buffer(const std::vector<T>& data, int type) : drawType(STATIC), arrayType(type) {
        gen_buffer(type);
        glBufferData(type, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
    }
    template VertexBuffer::Buffer::Buffer(const std::vector<int>& data, int type);
    template VertexBuffer::Buffer::Buffer(const std::vector<float>& data, int type);
    template VertexBuffer::Buffer::Buffer(const std::vector<Vector2f>& data, int type);
    template VertexBuffer::Buffer::Buffer(const std::vector<Vector3f>& data, int type);
    template VertexBuffer::Buffer::Buffer(const std::vector<Vector4f>& data, int type);

    VertexBuffer::Buffer::Buffer(size_t bytes, int type) : drawType(DYNAMIC), arrayType(type) {
        gen_buffer(type);
        glBufferData(type, bytes, nullptr, GL_DYNAMIC_DRAW);
    }

    VertexBuffer::Buffer::~Buffer(){
        glDeleteBuffers(1, &vbo);
    }

    void VertexBuffer::Buffer::bind(){ glBindBuffer(arrayType, vbo); }
    void VertexBuffer::Buffer::unbind(){ glBindBuffer(arrayType, 0); }

    // Static vars
    VertexBuffer::Buffer* VertexBuffer::tempBuffer = nullptr;

    // Private functions
    void VertexBuffer::buffer_sub_data(int type, unsigned long offset, unsigned long size, const void* ptr){ glBufferSubData(type, offset, size, ptr); }

    // Constructors
    VertexBuffer::VertexBuffer(){
        glGenVertexArrays(1, &vao);
    }

    VertexBuffer::~VertexBuffer(){
        for(auto* buf : buffers){
            delete buf;
        }
        buffers.clear();

        glDeleteVertexArrays(1, &vao);
    }

    // Functions
    void VertexBuffer::buffer(unsigned int index, const std::vector<int>& data, int type){
        bind();
        buffers.push_back(new Buffer(data, type));
        glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(int), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }
    
    void VertexBuffer::buffer(unsigned int index, const std::vector<float>& data, int type){
        bind();
        buffers.push_back(new Buffer(data, type));
        glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector2f>& data, int type){
        bind();
        buffers.push_back(new Buffer(data, type));
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, sizeof(Vector2f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector3f>& data, int type){
        bind();
        buffers.push_back(new Buffer(data, type));
        glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::buffer(unsigned int index, const std::vector<Vector4f>& data, int type){
        bind();
        buffers.push_back(new Buffer(data, type));
        glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4f), (void*)0);
        glEnableVertexAttribArray(index);
        unbind();
    }

    void VertexBuffer::start_buffer(const std::vector<float>& data, int type){
        bind();
        assert(!tempBuffer && "Buffer must be ended before starting another");
        tempBuffer = new Buffer(data, type);
    }

    void VertexBuffer::set_attribute(unsigned int index, int type, unsigned long count, unsigned long stride, unsigned long offset){
        assert(tempBuffer && "Buffer must be started before setting data");
        glVertexAttribPointer(index, count, type, GL_FALSE, stride, (void*)offset);
        glEnableVertexAttribArray(index);
    }

    void VertexBuffer::end_buffer(){
        assert(tempBuffer && "Buffer must be started before ending");
        unbind();
        buffers.push_back(tempBuffer);
        tempBuffer = nullptr;
    }

    void VertexBuffer::bind() const {
        glBindVertexArray(vao);
    }

    void VertexBuffer::unbind() const {
        glBindVertexArray(0);
    }
};