#pragma once

#include "glad/gl.h"

namespace Draft {
    /**
     * @brief A fixed-size Shader Storage Buffer Object bound to a fixed binding point `slot`.
     * `ShaderBuffer<T>(slot, data)` allocates `sizeof(T)` bytes. `set(const T&)` re-uploads the
     * whole struct.
     */
    template<typename T>
    class ShaderBuffer {
    private:
        // Variables
        unsigned int buffer = 0;
        unsigned int slot;

    public:
        // Constructors
        ShaderBuffer(unsigned int slot = 0, T* data = nullptr) : slot(slot) {
            glGenBuffers(1, &buffer);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T), data, GL_DYNAMIC_COPY);
        }

        ShaderBuffer(const ShaderBuffer& other) = delete;
        ShaderBuffer& operator=(const ShaderBuffer& other) = delete;

        ShaderBuffer(ShaderBuffer&& other) noexcept : buffer(other.buffer), slot(other.slot) {
            other.buffer = 0;
        }

        ShaderBuffer& operator=(ShaderBuffer&& other) noexcept {
            if(this != &other){
                if(buffer) glDeleteBuffers(1, &buffer);
                buffer = other.buffer;
                slot = other.slot;
                other.buffer = 0;
            }

            return *this;
        }

        ~ShaderBuffer(){
            if(buffer) glDeleteBuffers(1, &buffer);
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
}
