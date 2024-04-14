#pragma once

#include "draft/rendering/mesh.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/file_handle.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class Model {
    private:
        // Variables
        const bool reloadable;

        FileHandle handle;
        std::vector<Mesh> meshes;
        std::vector<std::unique_ptr<VertexBuffer>> buffers;

        // Private functions
        size_t component_byte_size(int type);
        void load_meshes(const FileHandle& handle);
        void buffer_meshes();

    public:
        // Constructors
        Model();
        Model(const FileHandle& handle);
        Model(const Model& other);

        // Operators
        Model& operator=(const Model& other);
        Model& operator=(Model&& other) noexcept;

        // Functions
        void render() const;
        void reload();
    };
};