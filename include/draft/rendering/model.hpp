#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/material.hpp"
#include "draft/rendering/mesh.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/file_handle.hpp"

#include <memory>
#include <vector>

namespace Draft {
    class Model {
    private:
        // Variables
        bool reloadable;

        FileHandle handle;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<int> meshToMaterialMap;
        std::vector<Matrix4> meshToMatrixMap;
        std::vector<std::unique_ptr<VertexBuffer>> buffers;
        std::vector<std::unique_ptr<Texture>> embeddedTextures;

        // Private functions
        size_t component_byte_size(int type);
        void load(const FileHandle& handle, bool avoidGL = false);
        void buffer_meshes();

    public:
        // Constructors
        Model();
        Model(const FileHandle& handle, bool avoidGL = false);
        Model(const Model& other);

        // Operators
        Model& operator=(const Model& other);
        Model& operator=(Model&& other) noexcept;

        // Functions
        void reload_materials();
        void render(Shader& shader, const Matrix4& matrix) const;
        void reload();
    };
};