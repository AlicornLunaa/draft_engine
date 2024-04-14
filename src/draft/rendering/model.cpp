#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "draft/math/vector2.hpp"
#include "draft/math/vector3.hpp"
#include "draft/rendering/mesh.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/vertex_buffer.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/logger.hpp"
#include "tiny_gltf.h"
#include "glad/gl.h"

#include <vector>
#include <memory>

namespace Draft {
    // Raw functions
    tinygltf::Model load_model(const FileHandle& handle){
        // Loads raw data into a model file from tinygltf
        tinygltf::TinyGLTF loader;
        tinygltf::Model mdl;
        std::string err, warn;
        bool res;

        if(handle.extension() == ".glb"){
            const auto& bytes = handle.read_bytes();
            res = loader.LoadBinaryFromMemory(&mdl, &err, &warn, reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
        } else {
            const auto& str = handle.read_string();
            auto basePath = std::filesystem::path("./assets");
            res = loader.LoadASCIIFromString(&mdl, &err, &warn, str.c_str(), str.length(), basePath);
        }

        if(!warn.empty()) {
            Logger::println(Level::WARNING, "Model", warn);
        }

        if(!err.empty()) {
            Logger::println(Level::SEVERE, "Model", err);
        }

        if(!res) {
            exit(0);
        }

        return mdl;
    }

    // Private functions
    size_t Model::component_byte_size(int type) {
        switch (type) {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            case TINYGLTF_COMPONENT_TYPE_BYTE:
                return sizeof(char);

            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            case TINYGLTF_COMPONENT_TYPE_SHORT:
                return sizeof(short);

            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            case TINYGLTF_COMPONENT_TYPE_INT:
                return sizeof(int);

            case TINYGLTF_COMPONENT_TYPE_FLOAT:
                return sizeof(float);

            case TINYGLTF_COMPONENT_TYPE_DOUBLE:
                return sizeof(double);

            default:
                return 0;
        }
    }
    
    void Model::load_meshes(const FileHandle& handle){
        // Loads meshes using TinyGLTF
        auto mdl = load_model(handle);
        meshes.clear();
        materials.clear();

        // Now file is loaded, parse data
        // Load materials
        for(auto& mat : mdl.materials){
            
        }

        // Load meshes
        std::vector<Vector3f> vertices{};
        std::vector<Vector2f> texCoord{};
        std::vector<int> indices{};

        for(auto& mesh : mdl.meshes){
            for(auto& primitive : mesh.primitives){
                {
                    // Load Vertices
                    const tinygltf::Accessor& accessor = mdl.accessors[primitive.attributes["POSITION"]];
                    const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];
                    const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; i++) {
                        // Positions are Vec3 components, so for each vec3 stride, offset for x, y, and z.
                        vertices.push_back({ positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] });
                    }
                }

                {
                    // Load UV
                    const tinygltf::Accessor& accessor = mdl.accessors[primitive.attributes["TEXCOORD_0"]];
                    const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];
                    const float* coords = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; i++) {
                        texCoord.push_back({ coords[i * 2 + 0], coords[i * 2 + 1] });
                    }
                }

                {
                    // Load Indices
                    const tinygltf::Accessor& accessor = mdl.accessors[primitive.indices];
                    const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
                    const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];
                    const unsigned short* indexArr = reinterpret_cast<const unsigned short*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
                    
                    for (size_t i = 0; i < accessor.count; i++) {
                        indices.push_back(indexArr[i]);
                    }
                }

                // Initialize mesh with data
                meshes.push_back({ vertices, indices, texCoord });
                vertices.clear();
                texCoord.clear();
                indices.clear();
            }
        }
    }

    void Model::buffer_meshes(){
        // Send the meshes to the OpenGL vertex buffer
        buffers.clear(); // Reset buffers

        // Generate VBO for each mesh
        for(auto& mesh : meshes){
            // Creates a VBO from the data
            buffers.push_back(std::make_unique<VertexBuffer>());
            auto& vbo = buffers.back();
            auto& vertices = mesh.get_vertices();

            // Send over vertices, the easy part
            vbo->buffer(0, vertices);

            // Send UV map coordinates if they exist, if not fill everything with 0
            if(mesh.is_uv_mapped()){
                vbo->buffer(1, mesh.get_tex_coords());
            } else {
                std::vector<Vector2f> tempUV(vertices.size(), { 0, 0 });
                vbo->buffer(1, tempUV);
            }

            // Send colors if they exist, if not fill everything with white
            if(mesh.is_color_mapped()){
                vbo->buffer(2, mesh.get_colors());
            } else {
                std::vector<Vector3f> tempColors(vertices.size(), { 1, 1, 1 });
                vbo->buffer(2, tempColors);
            }

            // Send indices if the mesh is indexed, nothing otherwise
            if(mesh.is_indexed()){
                vbo->buffer(3, mesh.get_indices(), GL_ELEMENT_ARRAY_BUFFER);
            }
        }
    }

    // Constructors
    Model::Model() : reloadable(false) {}

    Model::Model(const FileHandle& handle) : reloadable(true), handle(handle) {
        // Construct from file handle
        load_meshes(handle);
        buffer_meshes();
    }

    Model::Model(const Model& other) : reloadable(other.reloadable), meshes(other.meshes), handle(other.handle) {
        // Copy constructor
        buffer_meshes();
    }

    // Operators
    Model& Model::operator=(const Model& other){
        // Assignment operator
        handle = other.handle;
        meshes = other.meshes;
        buffer_meshes();
        return *this;
    }

    Model& Model::operator=(Model&& other) noexcept {
        // Move operator
        handle = other.handle;
        meshes = other.meshes;
        buffer_meshes();
        return *this;
    }

    // Functions
    void Model::render() const {
        // Draws the meshes
        for(size_t i = 0; i < buffers.size(); i++){
            auto& vbo = buffers[i];
            auto& mesh = meshes[i];

            // Render each vbo with mesh data
            vbo->bind();

            if(mesh.is_indexed()){
                glDrawElements(GL_TRIANGLES, mesh.get_indices().size(), GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, mesh.get_vertices().size());
            }

            vbo->unbind();
        }
    }

    void Model::reload(){
        if(!reloadable || !handle.exists()) return;
        load_meshes(handle);
        buffer_meshes();
    }
};