#pragma once

#include "glad/gl.h"

namespace Draft {
    // Shader storage buffer object implementation
    template<typename T>
    class ShaderBuffer {
    private:
        // Variables
        unsigned int buffer;
        unsigned int slot;

    public:
        // Constructors
        ShaderBuffer(unsigned int slot = 0, T* data = nullptr) : slot(slot) {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T), data, GL_DYNAMIC_COPY);
        }

        ~ShaderBuffer(){
            glDeleteBuffers(1, buffer);
        }

        // Functions
        void set(const T& data){
            bind();
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T), &data, GL_DYNAMIC_COPY);
            unbind();
        }

        void bind() const {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, buffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
        }

        void unbind() const {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        }
    };
};