#include "draft/rendering/mesh.hpp"
#include "glad/gl.h"

#include <cassert>

namespace Draft {
    // Constructors
    Mesh::Mesh(){}

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray) : indexedMesh(false), uvMapped(false), colorMapped(false) { // Raw mesh constructor
        vertices = vertexArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray) : indexedMesh(true), uvMapped(false), colorMapped(false) { // Indexed mesh
        vertices = vertexArray;
        indices = indexArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<Vector2f>& uvArray) : indexedMesh(false), uvMapped(true), colorMapped(false) { // Unindexed uv-mapped mesh
        vertices = vertexArray;
        texCoords = uvArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector2f>& uvArray) : indexedMesh(true), uvMapped(true), colorMapped(false) { // Indexed uv-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        texCoords = uvArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<Vector3f>& colorArray) : indexedMesh(false), uvMapped(false), colorMapped(true) { // Unindexed color-mapped mesh
        vertices = vertexArray;
        colors = colorArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector3f>& colorArray) : indexedMesh(true), uvMapped(false), colorMapped(true) { // Indexed color-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        colors = colorArray;
    }

    Mesh::Mesh(const std::vector<Vector3f>& vertexArray, const std::vector<int>& indexArray, const std::vector<Vector2f>& uvArray, const std::vector<Vector3f>& colorArray) : indexedMesh(true), uvMapped(true), colorMapped(true) { // Indexed uv-mapped color-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        texCoords = uvArray;
        colors = colorArray;
    }

    // Functions
    const std::vector<Vector3f>& Mesh::get_vertices() const {
        return vertices;
    }

    const std::vector<int>& Mesh::get_indices() const {
        assert(is_indexed() && "Mesh cannot be indexed");
        return indices;
    }

    const std::vector<Vector2f>& Mesh::get_tex_coords() const {
        assert(is_uv_mapped() && "Mesh is not UV-mapped");
        return texCoords;
    }

    const std::vector<Vector3f>& Mesh::get_colors() const {
        assert(is_color_mapped() && "Mesh is not color-mapped");
        return colors;
    }

    // Drawable mesh class
    DrawableMesh::DrawableMesh(){
        generate_vertex_array();
    }

    DrawableMesh::DrawableMesh(const Mesh& mesh) : mesh(mesh) {
        generate_vertex_array();
    }

    DrawableMesh::DrawableMesh(const DrawableMesh& other){
        mesh = other.mesh;
        generate_vertex_array();
    }

    DrawableMesh& DrawableMesh::operator=(const DrawableMesh& other){
        mesh = other.mesh;
        generate_vertex_array();
        return *this;
    }

    void DrawableMesh::generate_vertex_array(){
        // Positions (0), uv (1), colors (2) always exist as attributes
        auto& vertices = mesh.get_vertices();

        std::vector<std::variant<StaticBuffer, DynamicBuffer>> buffers{
            StaticBuffer({{0, GL_FLOAT, 3, sizeof(Vector3f), 0}}),
            StaticBuffer({{1, GL_FLOAT, 2, sizeof(Vector2f), 0}}),
            StaticBuffer({{2, GL_FLOAT, 3, sizeof(Vector3f), 0}}),
        };

        if(mesh.is_indexed()){
            buffers.push_back(StaticBuffer({}, GL_ELEMENT_ARRAY_BUFFER));
        }

        vao.create(buffers);
        vao.set_data(0, vertices);

        if(mesh.is_uv_mapped()){
            vao.set_data(1, mesh.get_tex_coords());
        } else {
            std::vector<Vector2f> tempUV(vertices.size(), Vector2f{0, 0});
            vao.set_data(1, tempUV);
        }

        if(mesh.is_color_mapped()){
            vao.set_data(2, mesh.get_colors());
        } else {
            std::vector<Vector3f> tempColors(vertices.size(), Vector3f{1, 1, 1});
            vao.set_data(2, tempColors);
        }

        if(mesh.is_indexed()){
            vao.set_data(3, mesh.get_indices());
        }
    }

    const VertexArray& DrawableMesh::get_vertex_array() const {
        return vao;
    }

    void DrawableMesh::render() const {
        vao.bind();

        if(mesh.is_indexed()){
            glDrawElements(GL_TRIANGLES, mesh.get_indices().size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh.get_vertices().size());
        }

        vao.unbind();
    }
}
