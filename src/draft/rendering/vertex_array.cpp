#include "draft/rendering/vertex_array.hpp"
#include <cassert>
#include <type_traits>

namespace Draft {
    // Private functions
    void VertexArray::bind_vbo(size_t index) const {
        const auto& buf = vbos[index];
        glBindBuffer(buf.glType, buf.vbo);
    }

    void VertexArray::unbind_vbo(size_t index) const {
        const auto& buf = vbos[index];
        glBindBuffer(buf.glType, 0);
    }

    void VertexArray::buffer_data(int glType, int glDataHint, unsigned long bytes, const void* ptr){ glBufferData(glType, bytes, ptr, glDataHint); }
    void VertexArray::buffer_sub_data(int glType, unsigned long offset, unsigned long bytes, const void* ptr){ glBufferSubData(glType, offset, bytes, ptr); }

    // Constructors
    VertexArray::VertexArray(const std::vector<BufferVariant>& buffers){
        create(buffers);
    }

    VertexArray::~VertexArray(){
        if(initialized){
            glDeleteVertexArrays(1, &vao);

            for(const auto& buf : vbos){
                glDeleteBuffers(1, &buf.vbo);
            }
        }
    }

    // Functions
    void VertexArray::bind() const {
        assert(initialized && "Vertex array not initialized");
        glBindVertexArray(vao);
    }

    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::create(const std::vector<BufferVariant>& buffers){
        // Generate vao and bind it
        glGenVertexArrays(1, &vao);
        initialized = true;
        bind();

        // Setup buffers
        for(const BufferVariant& buffer : buffers){
            std::visit([&](auto&& buf){
                // Common logic
                unsigned int vbo;
                glCreateBuffers(1, &vbo);
                glBindBuffer(buf.glType, vbo);

                // Variant-specific
                if constexpr (std::is_same_v<std::decay_t<decltype(buf)>, DynamicBuffer>){
                    // Dynamic only
                    glBufferData(buf.glType, buf.maxBytes, nullptr, buf.glDataHint);
                }

                // Setup attributes
                for(const BufferAttribute& attrib : buf.attribs){
                    // Enable each attrib
                    glEnableVertexAttribArray(attrib.index);
                    glVertexAttribDivisor(attrib.index, attrib.divisor);

                    // Setup for each type
                    if(attrib.type == GL_INT){
                        glVertexAttribIPointer(attrib.index, attrib.count, attrib.type, attrib.stride, (void*)attrib.offset);
                    } else {
                        glVertexAttribPointer(attrib.index, attrib.count, attrib.type, attrib.normalized, attrib.stride, (void*)attrib.offset);
                    }
                }

                // Cleanup
                glBindBuffer(buf.glType, 0);
                vbos.push_back({vbo, buffer});
            }, buffer);
        }

        // Cleanup
        unbind();
    }
};