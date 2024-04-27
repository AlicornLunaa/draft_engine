#include "draft/math/glm.hpp"
#include "draft/rendering/shape_batch.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "glad/gl.h"

using namespace std;

namespace Draft {
    // Static data
    array<Vector2f, 4> ShapeBatch::quadVertices = array<Vector2f, 4>({
        {0.f, 0.f}, // Top left
        {1.f, 0.f}, // Top right
        {1.f, 1.f}, // Bottom right
        {0.f, 1.f}  // Bottom left
    });

    // Private functions
    void ShapeBatch::generate_circle_vertices(vector<Vector2f>& vertices, vector<int>& indices, size_t segments){
        float pointsEveryDegree = 2*3.14f / segments;

        vertices.push_back({ 0, 0 });
        indices.push_back(0);

        for(float i = 0; i < 2*3.14f; i += pointsEveryDegree){
            vertices.push_back({ std::cos(i), std::sin(i) });
            indices.push_back(vertices.size() - 1);
        }
    }

    Matrix4 ShapeBatch::generate_transform_matrix(const Shape& quad) const {
        // Generates a transformation matrix for the given quad
        Matrix4 trans(1.f);
        trans = Math::translate(trans, { quad.position.x, quad.position.y, 0.f });
        trans = Math::scale(trans, { quad.size.x, quad.size.y, 1.f });
        trans = Math::rotate(trans, quad.rotation, { 0.f, 0.f, 1.f });
        return trans;
    }

    // Constructor
    ShapeBatch::ShapeBatch(){}

    // Functions
    void ShapeBatch::draw_circle(const Vector2f& position, float radius, size_t segments){
        // Add quad to the queue
        Shape shape {
            {},
            {},
            position,
            { radius, radius },
            currentColor,
            0.f,
            Primitive::CIRCLE
        };

        generate_circle_vertices(shape.vertices, shape.indices, segments);
        shapeQueue.emplace(shape);
    }

    void ShapeBatch::flush(){
        // Draws all the shapes to opengl
        vector<Vector3f> vertices;
        vector<int> indices;
        bool flushAgain = false; // Turns true if texture was changed and flush must happen again

        // Create vertex geometry
        while(!shapeQueue.empty()){
            auto& shape = shapeQueue.front();
            auto trans = generate_transform_matrix(shape);
            auto startIndex = vertices.size(); // Used for adding triangle indices

            // Vertices
            for(const auto& v : shape.vertices){
                // Adds the transformed vertex to the array
                auto transformedV = trans * Vector4f(v.x, v.y, 0, 1);
                vertices.push_back({ transformedV.x, transformedV.y, 0.f });
            }

            // Add indices
            for(const auto& i : shape.indices){
                indices.push_back(startIndex + i);
            }

            // Remove the quad because its data is stored in the vertices now
            shapeQueue.pop();
        }

        // Early exit if theres nothing to do
        if(vertices.size() <= 0)
            return;

        // Create vertex buffer and render it
        VertexBuffer vbo;
        vbo.buffer(0, vertices);
        vbo.buffer(1, indices, GL_ELEMENT_ARRAY_BUFFER);

        vbo.bind();
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        vbo.unbind();

        // Do it again for the rest of the quads
        if(flushAgain)
            flush();
    }
};