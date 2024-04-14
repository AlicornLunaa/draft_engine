#include "draft/math/vector2.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "draft/rendering/model.hpp"
#include "draft/math/vector3.hpp"
#include "draft/rendering/mesh.hpp"
#include "draft/util/logger.hpp"
#include "tiny_gltf.h"

#include <vector>

namespace Draft {
    // Private functions
    size_t Model::ComponentTypeByteSize(int type) {
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

    // Constructors
    Model::Model(){}

    Model::Model(const FileHandle& file){
        // Load filedata with tinygltf
        tinygltf::TinyGLTF loader;
        tinygltf::Model mdl;
        std::string err, warn;
        bool res;

        if(file.extension() == ".glb"){
            res = loader.LoadBinaryFromFile(&mdl, &err, &warn, file.get_path());
        } else {
            res = loader.LoadASCIIFromFile(&mdl, &err, &warn, file.get_path());
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

        // Now file is loaded, parse data
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
            }

            // Initialize mesh with data
            meshes.push_back(DrawableMesh({ vertices, indices, texCoord }));
            vertices.clear();
            indices.clear();
        }
    }

    void Model::render() const {
        // Draws the meshes
        for(auto& mesh : meshes){
            mesh.render();
        }
    }
};