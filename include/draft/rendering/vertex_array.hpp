#pragma once

#include "glad/gl.h"

#include <algorithm>
#include <array>
#include <variant>
#include <vector>

namespace Draft {
    enum class BufferType { STATIC, DYNAMIC };
    
    struct BufferAttribute {
        // Variables
        unsigned int index = 0;
        int type = GL_FLOAT;
        unsigned long count = 1;
        unsigned long stride = sizeof(float);
        unsigned long offset = 0;
        bool normalized = false;
        unsigned long divisor = 0;
    };

    struct RawBuffer {
        const BufferType type;
        const std::vector<BufferAttribute> attribs;
        const int glType;

        RawBuffer(BufferType type, const std::vector<BufferAttribute>& attribs, int glType)
            : type(type), attribs(attribs), glType(glType) {
        }
    };

    struct StaticBuffer : public RawBuffer {
        StaticBuffer(const std::vector<BufferAttribute>& attribs, int glType = GL_ARRAY_BUFFER)
            : RawBuffer(BufferType::STATIC, attribs, glType) {
        }

        // Only for parity with dynamicbuffer
        template<typename T>
        static StaticBuffer create(const std::vector<BufferAttribute>& attribs, int glType = GL_ARRAY_BUFFER){ return StaticBuffer(attribs, glType); }
    };

    struct DynamicBuffer : public RawBuffer {
        const unsigned long maxBytes;

        DynamicBuffer(size_t bytes, const std::vector<BufferAttribute>& attribs, int glType = GL_ARRAY_BUFFER)
            : RawBuffer(BufferType::DYNAMIC, attribs, glType), maxBytes(bytes) {
        }

        template<typename T>
        static DynamicBuffer create(size_t count, const std::vector<BufferAttribute>& attribs, int glType = GL_ARRAY_BUFFER){ return DynamicBuffer(count * sizeof(T), attribs, glType); }
    };

    class VertexArray {
    private:
        // Types
        using BufferVariant = std::variant<StaticBuffer, DynamicBuffer>;

        struct OpenGLBuffer {
            unsigned int vbo;
            unsigned long maxBytes = 0;
            BufferType type;
            int glType;

            OpenGLBuffer(unsigned int vbo, const BufferVariant& variant) : vbo(vbo) {
                // Save max bytes
                std::visit([&](auto&& buf){
                    // Common logic
                    type = buf.type;
                    glType = buf.glType;

                    // Variant-specific
                    if constexpr (std::is_same_v<std::decay_t<decltype(buf)>, DynamicBuffer>){
                        // Dynamic only
                        maxBytes = buf.maxBytes;
                    }
                }, variant);
            }
        };

        // Variables
        bool initialized = false;
        unsigned int vao = 0;
        std::vector<OpenGLBuffer> vbos; // First is ID, second is definition that created it

        // Functions
        void bind_vbo(size_t index) const;
        void unbind_vbo(size_t index) const;
        void buffer_data(int glType, unsigned long bytes, const void* ptr);
        void buffer_sub_data(int glType, unsigned long offset, unsigned long bytes, const void* ptr);

    public:
        // Constructors
        template<typename... Args>
        VertexArray(Args...  args){ create({ args... }); }

        VertexArray() = default;
        VertexArray(const std::vector<BufferVariant>& buffers);
        VertexArray(const VertexArray& other) = delete;
        ~VertexArray();

        // Ops
        VertexArray& operator=(const VertexArray& other) = delete;

        // Functions
        template<typename T>
        void set_data(size_t bufferIndex, const std::vector<T>& arr, unsigned long offset = 0){
            // Set data for each index
            auto& buf = vbos[bufferIndex];
            bind();
            bind_vbo(bufferIndex);

            // Check types, if dynamic use subdata and if static use regular
            switch(buf.type){
            case BufferType::DYNAMIC:
                buffer_sub_data(buf.glType, offset * sizeof(T), std::min(static_cast<unsigned long>(arr.size() * sizeof(T)), buf.maxBytes), (void*)arr.data());
                break;

            default:
            case BufferType::STATIC:
                buffer_data(buf.glType, arr.size() * sizeof(T), (void*)arr.data());
                break;
            }
        }

        template<typename T, size_t K>
        void set_data(size_t bufferIndex, const std::array<T, K>& arr, unsigned long offset = 0){
            // Set data for each index
            auto& buf = vbos[bufferIndex];
            bind();
            bind_vbo(bufferIndex);

            // Check types, if dynamic use subdata and if static use regular
            switch(buf.type){
            case BufferType::DYNAMIC:
                buffer_sub_data(buf.glType, offset * sizeof(T), std::min(static_cast<unsigned long>(arr.size() * sizeof(T)), buf.maxBytes), (void*)arr.data());
                break;

            default:
            case BufferType::STATIC:
                buffer_data(buf.glType, arr.size() * sizeof(T), (void*)arr.data());
                break;
            }
        }

        void bind() const;
        void unbind() const;
        void create(const std::vector<BufferVariant>& buffers);

        template<typename... Args>
        void create(Args... args){ create({ args... }); }
    };
};