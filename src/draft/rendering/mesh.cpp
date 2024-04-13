#include "draft/rendering/mesh.hpp"
#include "draft/math/vector3.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

#include <cassert>

using namespace std;

namespace Draft {
    // Mesh class
    // Constructors
    Mesh::Mesh(){}

    Mesh::Mesh(const vector<Vector3f>& vertexArray) : indexedMesh(false), uvMapped(false), colorMapped(false) { // Raw mesh constructor
        vertices = vertexArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<int>& indexArray) : indexedMesh(true), uvMapped(false), colorMapped(false) { // Indexed mesh
        vertices = vertexArray;
        indices = indexArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<Vector2f>& uvArray) : indexedMesh(false), uvMapped(true), colorMapped(false) { // Unindexed uv-mapped mesh
        vertices = vertexArray;
        texCoords = uvArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<int>& indexArray, const vector<Vector2f>& uvArray) : indexedMesh(true), uvMapped(true), colorMapped(false) { // Indexed uv-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        texCoords = uvArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<Vector3f>& colorArray) : indexedMesh(false), uvMapped(false), colorMapped(true) { // Unindexed color-mapped mesh
        vertices = vertexArray;
        colors = colorArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<int>& indexArray, const vector<Vector3f>& colorArray) : indexedMesh(true), uvMapped(false), colorMapped(true) { // Indexed color-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        colors = colorArray;
    }

    Mesh::Mesh(const vector<Vector3f>& vertexArray, const vector<int>& indexArray, const vector<Vector2f>& uvArray, const vector<Vector3f>& colorArray) : indexedMesh(true), uvMapped(true), colorMapped(true) { // Indexed uv-mapped color-mapped mesh
        vertices = vertexArray;
        indices = indexArray;
        texCoords = uvArray;
        colors = colorArray;
    }

    // Functions
    const vector<Vector3f>& Mesh::get_vertices() const {
        return vertices;
    }

    const vector<int>& Mesh::get_indices() const {
        assert(is_indexed() && "Mesh cannot be indexed");
        return indices;
    }

    const vector<Vector2f>& Mesh::get_tex_coords() const {
        assert(is_uv_mapped() && "Mesh is not UV-mapped");
        return texCoords;
    }

    const vector<Vector3f>& Mesh::get_colors() const {
        assert(is_color_mapped() && "Mesh is not color-mapped");
        return colors;
    }


    // Drawable mesh class
    DrawableMesh::DrawableMesh(){
        generate_vertex_buffer();
    }

    DrawableMesh::DrawableMesh(const Mesh& mesh) : mesh(mesh) {
        generate_vertex_buffer();
    }

    DrawableMesh::DrawableMesh(const DrawableMesh& other){
        mesh = other.mesh;
        generate_vertex_buffer();
    }

    DrawableMesh& DrawableMesh::operator=(const DrawableMesh& other){
        mesh = other.mesh;
        generate_vertex_buffer();
        return *this;
    }

    void DrawableMesh::generate_vertex_buffer(){
        // Creates a VBO from the data
        auto& vertices = mesh.get_vertices();

        vbo.buffer(0, vertices);

        if(mesh.is_uv_mapped()){
            vbo.buffer(1, mesh.get_tex_coords());
        } else {
            vector<Vector2f> tempUV{};

            for(size_t i = 0; i < vertices.size(); i++){
                tempUV.push_back({ 0, 0 });
            }

            vbo.buffer(1, tempUV);
        }

        if(mesh.is_color_mapped()){
            vbo.buffer(2, mesh.get_colors());
        } else {
            vector<Vector3f> tempColors{};

            for(size_t i = 0; i < vertices.size(); i++){
                tempColors.push_back({ 1, 1, 1 });
            }

            vbo.buffer(2, tempColors);
        }

        if(mesh.is_indexed()){
            vbo.buffer(3, mesh.get_indices(), GL_ELEMENT_ARRAY_BUFFER);
        }
    }

    const VertexBuffer& DrawableMesh::get_vertex_buffer() const {
        return vbo;
    }

    void DrawableMesh::render() const {
        vbo.bind();

        if(mesh.is_indexed()){
            glDrawElements(GL_TRIANGLES, mesh.get_indices().size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, mesh.get_vertices().size());
        }

        vbo.unbind();
    }
};