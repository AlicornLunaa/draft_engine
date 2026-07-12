#include "draft/rendering/image.hpp"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_JSON
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#include <nlohmann/json.hpp>
#include "stb_image.h"
#include "stb_image_write.h"
#include "tiny_gltf.h"

#include "draft/math/glm.hpp"
#include "draft/rendering/model.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/logger.hpp"
#include "glad/gl.h"

#include <filesystem>
#include <functional>
#include <stdexcept>
#include <vector>

namespace Draft {
    // TODO: Implement FileHandle stuff so it can load embedded and packed data
    tinygltf::Model Model::load_raw_model(const FileHandle& handle){
        tinygltf::TinyGLTF loader;
        tinygltf::Model mdl;
        std::string err, warn;
        bool ok;

        if(handle.extension() == ".glb"){
            const auto& bytes = handle.read_bytes();
            ok = loader.LoadBinaryFromMemory(&mdl, &err, &warn, reinterpret_cast<const unsigned char*>(bytes.data()), bytes.size());
        } else {
            const std::string str = handle.read_string();
            // std::filesystem::path::parent_path() directly, not FileHandle::parent()
            const std::string basePath = std::filesystem::path(handle.get_path()).parent_path().string();
            ok = loader.LoadASCIIFromString(&mdl, &err, &warn, str.c_str(), str.length(), basePath);
        }

        if(!warn.empty()) Logger::println(LogLevel::Warning, "Model", warn);
        if(!err.empty()) Logger::println(LogLevel::Severe, "Model", err);
        if(!ok) throw std::runtime_error("Model: failed to load glTF file '" + handle.get_path() + "'");

        return mdl;
    }

    void Model::load_materials(const FileHandle& handle, std::vector<Material3D>& materials, std::vector<std::shared_ptr<Texture>>& embeddedTextures, const tinygltf::Model& mdl){
        // See load_raw_model() for why this uses std::filesystem::path::parent_path() directly
        // rather than FileHandle::parent() (which throws on an empty parent path).
        const std::filesystem::path basePath = std::filesystem::path(handle.get_path()).parent_path();

        auto load_texture = [&](int index) -> Texture* {
            if(index == -1) return nullptr;

            const auto& texData = mdl.textures[index];
            const auto& img = mdl.images[texData.source];

            if(img.uri.empty()){
                // Embedded (already decoded by tinygltf's own stb_image usage)
                Image image({(unsigned)img.width, (unsigned)img.height}, channels_to_color_format(img.component), reinterpret_cast<const std::byte*>(img.image.data()));
                embeddedTextures.push_back(std::make_shared<Texture>(image));
            } else {
                // External file, resolved relative to the model's own directory
                FileHandle imageHandle = HostFileSystem().open((basePath / img.uri).string());
                embeddedTextures.push_back(std::make_shared<Texture>(imageHandle));
            }

            return embeddedTextures.back().get();
        };

        for(const auto& mat : mdl.materials){
            materials.push_back(Material3D{ mat.name });
            Material3D& material = materials.back();

            material.baseColor = { mat.pbrMetallicRoughness.baseColorFactor[0], mat.pbrMetallicRoughness.baseColorFactor[1], mat.pbrMetallicRoughness.baseColorFactor[2], mat.pbrMetallicRoughness.baseColorFactor[3] };
            material.emissiveFactor = { mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2] };
            material.metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
            material.roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
            material.normalScale = mat.normalTexture.scale;
            material.occlusionStrength = mat.occlusionTexture.strength;

            material.baseTexture = load_texture(mat.pbrMetallicRoughness.baseColorTexture.index);
            material.normalTexture = load_texture(mat.normalTexture.index);
            material.emissiveTexture = load_texture(mat.emissiveTexture.index);
            material.occlusionTexture = load_texture(mat.occlusionTexture.index);
            material.roughnessTexture = load_texture(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
        }

        // Dummy fallback material for primitives with primitive.material == -1
        materials.push_back(Material3D{ "missing_material_draft" });
    }

    // Reads N unsigned indices of the accessor's real component type (glTF indices are always
    // unsigned byte, short, or int), instead of assuming unsigned short regardless of what the
    // file actually declares.
    std::vector<int> Model::read_indices(const unsigned char* data, int componentType, size_t count){
        std::vector<int> indices;
        indices.reserve(count);

        switch(componentType){
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                for(size_t i = 0; i < count; i++) indices.push_back(data[i]);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                const unsigned short* arr = reinterpret_cast<const unsigned short*>(data);
                for(size_t i = 0; i < count; i++) indices.push_back(arr[i]);
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                const unsigned int* arr = reinterpret_cast<const unsigned int*>(data);
                for(size_t i = 0; i < count; i++) indices.push_back((int)arr[i]);
                break;
            }
            default:
                throw std::runtime_error("Model: index accessor has an unsupported component type");
        }

        return indices;
    }

    // Flattens every primitive of every glTF mesh into one entry each in meshes
    void Model::load_meshes(std::vector<Mesh>& meshes, std::vector<int>& meshToMaterialMap, std::vector<Matrix4>& meshToMatrixMap, std::vector<std::pair<size_t, size_t>>& meshPrimitiveRanges, const tinygltf::Model& mdl){
        std::vector<Vector3f> vertices;
        std::vector<Vector2f> texCoord;
        std::vector<int> indices;

        for(const auto& mesh : mdl.meshes){
            const size_t rangeStart = meshes.size();

            for(const auto& primitive : mesh.primitives){
                {
                    auto it = primitive.attributes.find("POSITION");
                    if(it == primitive.attributes.end())
                        throw std::runtime_error("Model: primitive is missing the required POSITION attribute");

                    const auto& accessor = mdl.accessors[it->second];
                    const auto& bufferView = mdl.bufferViews[accessor.bufferView];
                    const auto& buffer = mdl.buffers[bufferView.buffer];
                    const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                    for(size_t i = 0; i < accessor.count; i++)
                        vertices.push_back({ positions[i * 3 + 0], positions[i * 3 + 1], positions[i * 3 + 2] });
                }

                // TEXCOORD_0 is optional per the glTF spec, a primitive may have no UVs
                auto texCoordIt = primitive.attributes.find("TEXCOORD_0");
                if(texCoordIt != primitive.attributes.end()){
                    const auto& accessor = mdl.accessors[texCoordIt->second];
                    const auto& bufferView = mdl.bufferViews[accessor.bufferView];
                    const auto& buffer = mdl.buffers[bufferView.buffer];
                    const float* coords = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                    for(size_t i = 0; i < accessor.count; i++)
                        texCoord.push_back({ coords[i * 2 + 0], 1 - coords[i * 2 + 1] });
                }

                // indices is optional per the glTF spec, non-indexed geometry is legal
                if(primitive.indices >= 0){
                    const auto& accessor = mdl.accessors[primitive.indices];
                    const auto& bufferView = mdl.bufferViews[accessor.bufferView];
                    const auto& buffer = mdl.buffers[bufferView.buffer];
                    const unsigned char* base = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                    indices = read_indices(base, accessor.componentType, accessor.count);
                }

                if(indices.empty() && texCoord.empty()){
                    meshes.push_back(Mesh(vertices));
                } else if(indices.empty()){
                    meshes.push_back(Mesh(vertices, texCoord));
                } else if(texCoord.empty()){
                    meshes.push_back(Mesh(vertices, indices));
                } else {
                    meshes.push_back(Mesh(vertices, indices, texCoord));
                }

                meshToMaterialMap.push_back(primitive.material);
                meshToMatrixMap.push_back(Matrix4(1.f));
                vertices.clear();
                texCoord.clear();
                indices.clear();
            }

            meshPrimitiveRanges.push_back({ rangeStart, meshes.size() });
        }
    }

    // Computes a glTF node's local matrix. Either its raw matrix (glTF nodes specify *either*
    // matrix *or* T/R/S, never both) or a composed T * R * S, with rotation treated as the
    // quaternion [x, y, z, w] the glTF spec requires (not Euler angles).
    Matrix4 Model::compute_local_matrix(const std::vector<double>& translation, const std::vector<double>& rotation, const std::vector<double>& scale, const std::vector<double>& matrix){
        if(matrix.size() == 16){
            // glTF node matrices are column-major, matching glm's own Matrix4 layout
            Matrix4 result;
            for(int col = 0; col < 4; col++)
                for(int row = 0; row < 4; row++)
                    result[col][row] = (float)matrix[col * 4 + row];
            return result;
        }

        Matrix4 result(1.f);

        if(translation.size() == 3)
            result = Math::translate(result, { (float)translation[0], (float)translation[1], (float)translation[2] });

        if(rotation.size() == 4){
            Quaternion q((float)rotation[3], (float)rotation[0], (float)rotation[1], (float)rotation[2]);
            result = result * Math::mat4_cast(q);
        }

        if(scale.size() == 3)
            result = Math::scale(result, { (float)scale[0], (float)scale[1], (float)scale[2] });

        return result;
    }

    // Walks the node hierarchy from its root(s), composing each node's world matrix as
    // parentWorld * localMatrix, and applies the result to every primitive in that node's mesh's
    // flattened range (see load_meshes()).
    void Model::load_nodes(std::vector<Matrix4>& meshToMatrixMap, const std::vector<std::pair<size_t, size_t>>& meshPrimitiveRanges, const tinygltf::Model& mdl){
        std::function<void(int, const Matrix4&)> visit = [&](int nodeIndex, const Matrix4& parentWorld){
            const auto& node = mdl.nodes[nodeIndex];
            Matrix4 world = parentWorld * compute_local_matrix(node.translation, node.rotation, node.scale, node.matrix);

            if(node.mesh >= 0){
                const auto& range = meshPrimitiveRanges[node.mesh];
                for(size_t i = range.first; i < range.second; i++)
                    meshToMatrixMap[i] = world;
            }

            for(int child : node.children)
                visit(child, world);
        };

        if(!mdl.scenes.empty()){
            const int sceneIndex = (mdl.defaultScene >= 0) ? mdl.defaultScene : 0;
            for(int root : mdl.scenes[sceneIndex].nodes)
                visit(root, Matrix4(1.f));
        } else {
            std::vector<bool> hasParent(mdl.nodes.size(), false);
            for(const auto& node : mdl.nodes)
                for(int child : node.children)
                    hasParent[child] = true;

            for(size_t i = 0; i < mdl.nodes.size(); i++)
                if(!hasParent[i])
                    visit((int)i, Matrix4(1.f));
        }
    }

    // Constructors
    Model::Model() : reloadable(false) {}

    Model::Model(const FileHandle& handle) : reloadable(true), handle(handle) {
        load(handle);
    }

    Model::Model(const Model& other) : reloadable(other.reloadable), handle(other.handle), meshes(other.meshes), materials(other.materials), meshToMaterialMap(other.meshToMaterialMap), meshToMatrixMap(other.meshToMatrixMap), embeddedTextures(other.embeddedTextures) {
    }

    // Operators
    Model& Model::operator=(const Model& other){
        reloadable = other.reloadable;
        handle = other.handle;
        meshes = other.meshes;
        materials = other.materials;
        meshToMaterialMap = other.meshToMaterialMap;
        meshToMatrixMap = other.meshToMatrixMap;
        embeddedTextures = other.embeddedTextures;
        return *this;
    }

    Model& Model::operator=(Model&& other) noexcept {
        reloadable = other.reloadable;
        handle = other.handle;
        meshes = std::move(other.meshes);
        materials = std::move(other.materials);
        meshToMaterialMap = std::move(other.meshToMaterialMap);
        meshToMatrixMap = std::move(other.meshToMatrixMap);
        embeddedTextures = std::move(other.embeddedTextures);
        return *this;
    }

    void Model::load(const FileHandle& handle){
        materials.clear();
        meshes.clear();
        meshToMaterialMap.clear();
        meshToMatrixMap.clear();
        embeddedTextures.clear();

        tinygltf::Model mdl = load_raw_model(handle);

        std::vector<Mesh> cpuMeshes;
        std::vector<std::pair<size_t, size_t>> meshPrimitiveRanges;

        load_materials(handle, materials, embeddedTextures, mdl);
        load_meshes(cpuMeshes, meshToMaterialMap, meshToMatrixMap, meshPrimitiveRanges, mdl);
        load_nodes(meshToMatrixMap, meshPrimitiveRanges, mdl);

        meshes.reserve(cpuMeshes.size());
        for(const auto& mesh : cpuMeshes)
            meshes.emplace_back(mesh);
    }

    // Functions
    void Model::reload_materials(){
        if(!reloadable) return;

        tinygltf::Model mdl = load_raw_model(*handle);
        materials.clear();
        load_materials(*handle, materials, embeddedTextures, mdl);
    }

    void Model::render(const Shader& shader, const Matrix4& modelMatrix) const {
        for(size_t i = 0; i < meshes.size(); i++){
            const auto& mesh = meshes[i];
            const auto& material = materials[(meshToMaterialMap[i] == -1) ? (materials.size() - 1) : meshToMaterialMap[i]];
            const auto& matrix = meshToMatrixMap[i];

            material.apply(shader);
            shader.set_uniform("model", modelMatrix * matrix);
            mesh.render();
        }
    }

    void Model::reload(){
        if(!reloadable || !handle->exists()) return;
        load(*handle);
    }
}
