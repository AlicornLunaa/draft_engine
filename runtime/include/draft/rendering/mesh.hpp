#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/vertex_array.hpp"

#include <vector>

namespace Draft {
    /**
     * @brief Pure CPU-side mesh data, vertices plus optional indices/UVs/colors. No GL
     * dependency of its own
     */
    class Mesh {
    private:
        // Variables
        std::vector<Vector3f> vertices{};
        std::vector<Vector2f> texCoords{};
        std::vector<int> indices{};
        std::vector<Vector3f> colors{};

        bool uvMapped = false; // Whether or not to use texture coordinates
        bool indexedMesh = false; // Whether or not to use indices
        bool colorMapped = false; // Whether or not to use colors coordinates

    public:
        // Constructors
        Mesh();
        Mesh(const std::vector<Vector3f>& vertexArray); // Raw mesh constructor
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray); // Indexed mesh
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<Vector2f>& uvArray); // Unindexed uv-mapped mesh
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector2f>& uvArray); // Indexed uv-mapped mesh
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<Vector3f>& colorArray); // Unindexed color-mapped mesh
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector3f>& colorArray); // Indexed color-mapped mesh
        Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector2f>& uvArray, const std::vector<Vector3f>& colorArray); // Indexed uv-mapped color-mapped mesh

        // Functions
        inline bool is_uv_mapped() const { return uvMapped; }
        inline bool is_indexed() const { return indexedMesh; }
        inline bool is_color_mapped() const { return colorMapped; }

        const std::vector<Vector3f>& get_vertices() const;
        const std::vector<int>& get_indices() const;
        const std::vector<Vector2f>& get_tex_coords() const;
        const std::vector<Vector3f>& get_colors() const;
    };

    /**
     * @brief GPU-side counterpart to Mesh, owns a VertexArray built from the mesh's vertex/
     * uv/color/index data (positions at attribute 0, uv at 1, colors at 2, matching
     * assets/shaders/mesh/vertex.glsl) and can render it. Do not construct before an OpenGL
     * context was established.
     */
    class DrawableMesh {
    private:
        // Variables
        Mesh mesh;
        VertexArray vao;

        // Private functions
        void generate_vertex_array();

    public:
        // Constructors
        DrawableMesh();
        DrawableMesh(const Mesh& mesh);
        DrawableMesh(const DrawableMesh& other);

        // Operators
        DrawableMesh& operator=(const DrawableMesh& other);
        operator Mesh& () { return mesh; }

        // Functions
        const VertexArray& get_vertex_array() const;
        void render() const;
    };
}
