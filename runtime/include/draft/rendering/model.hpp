#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/material.hpp"
#include "draft/rendering/mesh.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/file_handle.hpp"

#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace tinygltf {
    class Model;
}

namespace Draft {
    /**
     * @brief A loaded glTF (.gltf/.glb) scene
     */
    class Model {
    public:
        // Constructors
        Model();
        Model(const FileHandle& handle);
        Model(const Model& other);

        // Operators
        Model& operator=(const Model& other);
        Model& operator=(Model&& other) noexcept;

        // Functions
        void reload_materials();
        void render(const Shader& shader, const Matrix4& matrix) const;
        void reload();

    private:
        friend struct ModelTestAccess;

        /**
         * @brief Computes a glTF node's local matrix
         */
        static Matrix4 compute_local_matrix(const std::vector<double>& translation, const std::vector<double>& rotation, const std::vector<double>& scale, const std::vector<double>& matrix);

        /**
         * @brief Reads @p count unsigned indices starting at @p data, using @p componentType
         * (one of glTF's unsigned component types byte, short, or int) rather than assuming a
         * fixed width.
         */
        static std::vector<int> read_indices(const unsigned char* data, int componentType, size_t count);

        static tinygltf::Model load_raw_model(const FileHandle& handle);
        static void load_materials(const FileHandle& handle, std::vector<Material3D>& materials, std::vector<std::shared_ptr<Texture>>& embeddedTextures, const tinygltf::Model& mdl);
        static void load_meshes(std::vector<Mesh>& meshes, std::vector<int>& meshToMaterialMap, std::vector<Matrix4>& meshToMatrixMap, std::vector<std::pair<size_t, size_t>>& meshPrimitiveRanges, const tinygltf::Model& mdl);
        static void load_nodes(std::vector<Matrix4>& meshToMatrixMap, const std::vector<std::pair<size_t, size_t>>& meshPrimitiveRanges, const tinygltf::Model& mdl);

        void load(const FileHandle& handle);

        // Variables
        bool reloadable;

        std::optional<FileHandle> handle; // Unset for procedurally-built models
        std::vector<DrawableMesh> meshes;
        std::vector<Material3D> materials;
        std::vector<int> meshToMaterialMap;
        std::vector<Matrix4> meshToMatrixMap;
        std::vector<std::shared_ptr<Texture>> embeddedTextures;
    };
}
