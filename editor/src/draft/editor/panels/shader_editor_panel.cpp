#include "draft/editor/panels/shader_editor_panel.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets.hpp"
#include "draft/rendering/image.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstdint>
#include <exception>

namespace Draft {
    namespace {
        // Lazily-constructed, process-lifetime fallback for an unassigned sampler2D uniform.
        Texture& fallback_white_texture(){
            static Texture* tex = new Texture(Image({1, 1}, {1, 1, 1, 1}));
            return *tex;
        }

        int component_count(ShaderDataType type){
            switch(type){
                case ShaderDataType::Float:
                case ShaderDataType::Int:
                case ShaderDataType::UInt:
                case ShaderDataType::Bool:
                    return 1;
                case ShaderDataType::Vec2:
                case ShaderDataType::IVec2:
                case ShaderDataType::UVec2:
                    return 2;
                case ShaderDataType::Vec3:
                case ShaderDataType::IVec3:
                case ShaderDataType::UVec3:
                    return 3;
                case ShaderDataType::Vec4:
                case ShaderDataType::IVec4:
                case ShaderDataType::UVec4:
                    return 4;
                default:
                    return 0; // Mat2/Mat3/Mat4/Sampler2D/Unknown - not a plain N-float value
            }
        }

        bool name_contains(const std::string& name, const char* needle){
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
            return lower.find(needle) != std::string::npos;
        }

        std::array<std::array<float, 4>, 4> default_attribute_values(const ShaderAttribute& attribute){
            std::array<std::array<float, 4>, 4> values{};
            for(auto& v : values)
                v = {0.f, 0.f, 0.f, 0.f};

            int count = component_count(attribute.type);
            if(count < 2)
                return values;

            static constexpr float quad[4][2] = {{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f, 0.5f}, {-0.5f, 0.5f}};
            static constexpr float uv[4][2] = {{0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}};

            // Location 0 is, by convention across every shader shipped with the engine (see
            // assets/shaders/*/vertex.glsl), always the position attribute.
            bool looksLikePosition = attribute.location == 0 || name_contains(attribute.name, "pos");
            bool looksLikeUV = count == 2 && (name_contains(attribute.name, "uv") || name_contains(attribute.name, "tex"));
            bool looksLikeColor = name_contains(attribute.name, "col");
            bool looksLikeNormal = count >= 3 && name_contains(attribute.name, "normal");

            for(int i = 0; i < 4; i++){
                if(looksLikePosition){
                    values[i][0] = quad[i][0];
                    values[i][1] = quad[i][1];
                    if(count == 4)
                        values[i][3] = 1.f;
                } else if(looksLikeUV){
                    values[i][0] = uv[i][0];
                    values[i][1] = uv[i][1];
                } else if(looksLikeColor){
                    for(int c = 0; c < count; c++)
                        values[i][c] = 1.f;
                } else if(looksLikeNormal){
                    values[i][2] = 1.f;
                }
            }

            return values;
        }

        std::array<float, 4> default_uniform_value(const ShaderUniform& uniform){
            std::array<float, 4> value{0.f, 0.f, 0.f, 0.f};
            int count = component_count(uniform.type);

            if(name_contains(uniform.name, "color") || name_contains(uniform.name, "tint"))
                for(int c = 0; c < count; c++)
                    value[c] = 1.f;

            return value;
        }
    }

    ShaderEditorPanelSystem::ShaderEditorPanelSystem(EditorApplication& app)
        : m_app(app), m_previewTarget(FramebufferProperties{.size = {1024, 1024}})
    {
    }

    void ShaderEditorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.shaderEditorPanelVisible)
            return;

        std::string title = "Shader Editor";
        if(m_app.shaderEditorAssetKey)
            title += " - " + std::filesystem::path(*m_app.shaderEditorAssetKey).filename().string();
        title += "###ShaderEditor";

        ImGui::SetNextWindowSize({460, 640}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin(title.c_str(), &m_app.shaderEditorPanelVisible)){
            if(!m_app.shaderEditorAssetKey){
                ImGui::TextDisabled("Double click a shader asset in the Asset Browser to edit it.");
            } else {
                if(m_loadedKey != *m_app.shaderEditorAssetKey)
                    load_asset(*m_app.shaderEditorAssetKey);

                bool changed = false;
                if(m_vertexWatcher && m_vertexWatcher->poll())
                    changed = true;
                if(m_fragmentWatcher && m_fragmentWatcher->poll())
                    changed = true;
                if(changed)
                    compile_and_reflect();

                if(!m_compileError.empty())
                    ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_compileError.c_str());
                else if(!m_shader)
                    ImGui::TextDisabled("Compiling...");

                if(!m_storageBlocks.empty()){
                    std::string names;
                    for(const ShaderStorageBlock& block : m_storageBlocks){
                        if(!names.empty())
                            names += ", ";
                        names += block.name + " (binding " + std::to_string(block.binding) + ")";
                    }

                    ImGui::TextColored(ImVec4(1.f, 0.8f, 0.3f, 1.f), "Uses shader storage buffer(s) not fed by preview data: %s. The preview may render nothing.", names.c_str());
                }

                constexpr float dataHeight = 320.f;
                float previewHeight = std::max(ImGui::GetContentRegionAvail().y - dataHeight - ImGui::GetStyle().ItemSpacing.y, 100.f);

                draw_preview(previewHeight);
                ImGui::Separator();

                ImGui::BeginChild("ShaderEditorData");
                if(ImGui::CollapsingHeader("Vertex Attributes", ImGuiTreeNodeFlags_DefaultOpen))
                    draw_attribute_editors();
                if(ImGui::CollapsingHeader("Uniforms", ImGuiTreeNodeFlags_DefaultOpen))
                    draw_uniform_editors();

                ImGui::Separator();
                if(ImGui::Button("Save Mock Data"))
                    save_mock_data();
                ImGui::EndChild();
            }
        }

        ImGui::End();
    }

    void ShaderEditorPanelSystem::load_asset(const std::string& key){
        m_loadedKey = key;
        m_shader.reset();
        m_compileError.clear();
        m_attributes.clear();
        m_uniforms.clear();
        m_attributeValues.clear();
        m_uniformValues.clear();
        m_samplerValues.clear();
        m_storageBlocks.clear();
        m_previewVao.reset();
        m_vaoAttributeOrder.clear();
        m_vertexWatcher.reset();
        m_fragmentWatcher.reset();

        if(!m_app.has_project())
            return;

        std::filesystem::path root = m_app.project()->root();
        m_vertexWatcher.emplace(root / key / "vertex.glsl");
        m_fragmentWatcher.emplace(root / key / "fragment.glsl");

        load_mock_data();
        compile_and_reflect();
    }

    void ShaderEditorPanelSystem::compile_and_reflect(){
        if(!m_app.has_project())
            return;

        std::filesystem::path root = m_app.project()->root();
        FileHandle vertexHandle = HostFileSystem().open(root / m_loadedKey / "vertex.glsl");
        FileHandle fragmentHandle = HostFileSystem().open(root / m_loadedKey / "fragment.glsl");

        try {
            Shader candidate(vertexHandle, fragmentHandle);
            m_shader.emplace(std::move(candidate));
            m_compileError.clear();

            m_attributes = m_shader->reflect_attributes();
            m_uniforms = m_shader->reflect_uniforms();
            m_storageBlocks = m_shader->reflect_storage_blocks();
            reconcile_mock_data();
            rebuild_preview_mesh();
        } catch(const std::exception& e){
            // Shader's own constructor already logged the GL compile/link error, this is just
            // kept around for the inline banner. Last-good m_shader/attributes/uniforms/mesh
            // are left untouched so the preview keeps showing whatever last compiled.
            m_compileError = e.what();
        }
    }

    void ShaderEditorPanelSystem::reconcile_mock_data(){
        for(const ShaderAttribute& attribute : m_attributes){
            if(component_count(attribute.type) == 0)
                continue; // Unsupported attribute type for the dummy mesh (matrix/sampler/...)

            if(!m_attributeValues.contains(attribute.name))
                m_attributeValues[attribute.name] = default_attribute_values(attribute);
        }

        for(const ShaderUniform& uniform : m_uniforms){
            if(uniform.type == ShaderDataType::Sampler2D){
                m_samplerValues.try_emplace(uniform.name);
                continue;
            }

            if(component_count(uniform.type) == 0)
                continue; // Matrix (auto identity) or unrecognized type - nothing to edit

            if(!m_uniformValues.contains(uniform.name))
                m_uniformValues[uniform.name] = default_uniform_value(uniform);
        }
    }

    const ShaderAttribute* ShaderEditorPanelSystem::find_attribute(const std::string& name) const {
        for(const ShaderAttribute& attribute : m_attributes)
            if(attribute.name == name)
                return &attribute;

        return nullptr;
    }

    void ShaderEditorPanelSystem::rebuild_preview_mesh(){
        m_previewVao.reset();
        m_vaoAttributeOrder.clear();

        std::vector<std::variant<StaticBuffer, DynamicBuffer>> buffers;

        for(const ShaderAttribute& attribute : m_attributes){
            int count = component_count(attribute.type);
            if(count == 0)
                continue;

            buffers.push_back(StaticBuffer({{
                static_cast<unsigned int>(attribute.location),
                GL_FLOAT,
                static_cast<unsigned long>(count),
                static_cast<unsigned long>(count) * sizeof(float),
                0
            }}));

            m_vaoAttributeOrder.push_back(attribute.name);
        }

        if(m_vaoAttributeOrder.empty())
            return;

        m_previewVao.emplace();
        m_previewVao->create(buffers);

        for(size_t i = 0; i < m_vaoAttributeOrder.size(); i++){
            const ShaderAttribute* attribute = find_attribute(m_vaoAttributeOrder[i]);
            m_previewVao->set_data(i, flatten_attribute(attribute->name, component_count(attribute->type)));
        }
    }

    std::vector<float> ShaderEditorPanelSystem::flatten_attribute(const std::string& name, int componentCount) const {
        std::vector<float> data;
        data.reserve(4 * static_cast<size_t>(componentCount));

        auto it = m_attributeValues.find(name);
        for(int vertex = 0; vertex < 4; vertex++){
            for(int c = 0; c < componentCount; c++)
                data.push_back(it != m_attributeValues.end() ? it->second[vertex][c] : 0.f);
        }

        return data;
    }

    void ShaderEditorPanelSystem::upload_attribute(const ShaderAttribute& attribute){
        if(!m_previewVao)
            return;

        auto it = std::find(m_vaoAttributeOrder.begin(), m_vaoAttributeOrder.end(), attribute.name);
        if(it == m_vaoAttributeOrder.end())
            return;

        size_t index = static_cast<size_t>(std::distance(m_vaoAttributeOrder.begin(), it));
        m_previewVao->set_data(index, flatten_attribute(attribute.name, component_count(attribute.type)));
    }

    void ShaderEditorPanelSystem::apply_uniforms() const {
        unsigned int nextTextureUnit = 0;

        for(const ShaderUniform& uniform : m_uniforms){
            switch(uniform.type){
                case ShaderDataType::Mat2:
                    m_shader->set_uniform(uniform.name, Matrix2(1.f));
                    break;
                case ShaderDataType::Mat3:
                    m_shader->set_uniform(uniform.name, Matrix3(1.f));
                    break;
                case ShaderDataType::Mat4:
                    m_shader->set_uniform(uniform.name, Matrix4(1.f));
                    break;
                case ShaderDataType::Sampler2D: {
                    unsigned int unit = nextTextureUnit++;
                    auto it = m_samplerValues.find(uniform.name);

                    if(it != m_samplerValues.end() && it->second.is_valid())
                        it->second->bind(unit);
                    else
                        fallback_white_texture().bind(unit);

                    m_shader->set_uniform(uniform.name, static_cast<int>(unit));
                    break;
                }
                default: {
                    auto it = m_uniformValues.find(uniform.name);
                    if(it == m_uniformValues.end())
                        break;

                    const MockValue& v = it->second;
                    switch(uniform.type){
                        case ShaderDataType::Float: m_shader->set_uniform(uniform.name, v[0]); break;
                        case ShaderDataType::Vec2: m_shader->set_uniform(uniform.name, Vector2f(v[0], v[1])); break;
                        case ShaderDataType::Vec3: m_shader->set_uniform(uniform.name, Vector3f(v[0], v[1], v[2])); break;
                        case ShaderDataType::Vec4: m_shader->set_uniform(uniform.name, Vector4f(v[0], v[1], v[2], v[3])); break;
                        case ShaderDataType::Int: m_shader->set_uniform(uniform.name, static_cast<int>(v[0])); break;
                        case ShaderDataType::IVec2: m_shader->set_uniform(uniform.name, Vector2i(static_cast<int>(v[0]), static_cast<int>(v[1]))); break;
                        case ShaderDataType::IVec3: m_shader->set_uniform(uniform.name, Vector3i(static_cast<int>(v[0]), static_cast<int>(v[1]), static_cast<int>(v[2]))); break;
                        case ShaderDataType::IVec4: m_shader->set_uniform(uniform.name, Vector4i(static_cast<int>(v[0]), static_cast<int>(v[1]), static_cast<int>(v[2]), static_cast<int>(v[3]))); break;
                        case ShaderDataType::UInt: m_shader->set_uniform(uniform.name, static_cast<unsigned int>(std::max(0.f, v[0]))); break;
                        case ShaderDataType::UVec2: m_shader->set_uniform(uniform.name, Vector2u(static_cast<unsigned int>(std::max(0.f, v[0])), static_cast<unsigned int>(std::max(0.f, v[1])))); break;
                        case ShaderDataType::UVec3: m_shader->set_uniform(uniform.name, Vector3u(static_cast<unsigned int>(std::max(0.f, v[0])), static_cast<unsigned int>(std::max(0.f, v[1])), static_cast<unsigned int>(std::max(0.f, v[2])))); break;
                        case ShaderDataType::UVec4: m_shader->set_uniform(uniform.name, Vector4u(static_cast<unsigned int>(std::max(0.f, v[0])), static_cast<unsigned int>(std::max(0.f, v[1])), static_cast<unsigned int>(std::max(0.f, v[2])), static_cast<unsigned int>(std::max(0.f, v[3])))); break;
                        case ShaderDataType::Bool: m_shader->set_uniform(uniform.name, v[0] != 0.f); break;
                        default: break;
                    }
                    break;
                }
            }
        }
    }

    std::filesystem::path ShaderEditorPanelSystem::mock_data_path() const {
        std::string sanitized = m_loadedKey;
        std::replace(sanitized.begin(), sanitized.end(), '/', '_');
        std::replace(sanitized.begin(), sanitized.end(), '\\', '_');

        return m_app.project()->root() / ".draft-editor" / "shader_previews" / (sanitized + ".json");
    }

    void ShaderEditorPanelSystem::load_mock_data(){
        FileHandle handle = HostFileSystem().open(mock_data_path());
        if(!handle.exists())
            return;

        try {
            JSON json(handle);

            if(json.contains("attributes")){
                for(auto entry : json["attributes"].items()){
                    const JSON& perVertex = static_cast<const JSON&>(entry.value());
                    std::array<MockValue, 4> values{};
                    for(auto& v : values)
                        v = {0.f, 0.f, 0.f, 0.f};

                    for(size_t i = 0; i < 4 && i < perVertex.size(); i++){
                        const JSON& comps = perVertex[i];
                        for(size_t c = 0; c < 4 && c < comps.size(); c++)
                            values[i][c] = comps[c].get<float>();
                    }

                    m_attributeValues[entry.key()] = values;
                }
            }

            if(json.contains("uniforms")){
                for(auto entry : json["uniforms"].items()){
                    const JSON& comps = static_cast<const JSON&>(entry.value());
                    MockValue value{0.f, 0.f, 0.f, 0.f};
                    for(size_t c = 0; c < 4 && c < comps.size(); c++)
                        value[c] = comps[c].get<float>();

                    m_uniformValues[entry.key()] = value;
                }
            }

            if(json.contains("samplers")){
                for(auto entry : json["samplers"].items()){
                    std::string assetKey = entry.value().get<std::string>();
                    if(!assetKey.empty())
                        m_samplerValues[entry.key()] = m_app.assets.get<Texture>(assetKey);
                }
            }

            m_wireframe = json.value("wireframe", false);
        } catch(const std::exception& e){
            Logger::println(LogLevel::Warning, "ShaderEditor", "Failed to load mock data for " + m_loadedKey + ": " + e.what());
        }
    }

    void ShaderEditorPanelSystem::save_mock_data() const {
        if(!m_app.has_project())
            return;

        JSON json = JSON::object();

        JSON attributesJson = JSON::object();
        for(const ShaderAttribute& attribute : m_attributes){
            int count = component_count(attribute.type);
            auto it = m_attributeValues.find(attribute.name);
            if(count == 0 || it == m_attributeValues.end())
                continue;

            JSON perVertex = JSON::array();
            for(const MockValue& v : it->second){
                JSON comps = JSON::array();
                for(int c = 0; c < count; c++)
                    comps.push_back(v[c]);
                perVertex.push_back(comps);
            }

            attributesJson[attribute.name] = perVertex;
        }
        json["attributes"] = attributesJson;

        JSON uniformsJson = JSON::object();
        for(const ShaderUniform& uniform : m_uniforms){
            int count = component_count(uniform.type);
            auto it = m_uniformValues.find(uniform.name);
            if(count == 0 || it == m_uniformValues.end())
                continue;

            JSON comps = JSON::array();
            for(int c = 0; c < count; c++)
                comps.push_back(it->second[c]);

            uniformsJson[uniform.name] = comps;
        }
        json["uniforms"] = uniformsJson;

        JSON samplersJson = JSON::object();
        for(const auto& [name, resource] : m_samplerValues)
            samplersJson[name] = m_app.assets.key_for<Texture>(resource).value_or("");
        json["samplers"] = samplersJson;

        json["wireframe"] = m_wireframe;

        HostFileSystem().open(mock_data_path()).write_string(json.dump(4));
    }

    void ShaderEditorPanelSystem::draw_preview(float availableHeight){
        ImGui::Checkbox("Wireframe", &m_wireframe);

        float rowWidth = ImGui::GetContentRegionAvail().x;
        float imageHeight = availableHeight - ImGui::GetFrameHeightWithSpacing();
        float squareSize = std::clamp(std::min(rowWidth, imageHeight), 64.f, 1024.f);

        m_previewTarget.begin();

        if(m_shader && m_previewVao && !m_vaoAttributeOrder.empty()){
            // Same convention as SpriteCollection::flush(): this bypasses Renderer::set_state(),
            // so depth/blend must be put back to RenderState{}'s defaults (depthTest on, blend
            // off) before returning, or the next real pass's diffed set_state() call could wrongly
            // skip reapplying them and inherit whatever's left here.
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            m_shader->bind();
            apply_uniforms();

            VertexArray::set_wireframe(m_wireframe);
            m_previewVao->draw_arrays(GL_TRIANGLE_FAN, 0, 4);
            VertexArray::set_wireframe(false);

            m_shader->unbind();

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        m_previewTarget.end();

        float centerOffset = (rowWidth - squareSize) * 0.5f;
        if(centerOffset > 0.f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);

        ImTextureID textureId = static_cast<ImTextureID>(static_cast<intptr_t>(m_previewTarget.get_texture().get_texture_handle()));
        ImGui::Image(textureId, ImVec2(squareSize, squareSize), ImVec2(0, 1), ImVec2(1, 0));
    }

    void ShaderEditorPanelSystem::draw_attribute_editors(){
        if(m_attributes.empty())
            ImGui::TextDisabled("No active vertex attributes.");

        ImGui::TextDisabled("Fed to the vertex shader as a 4-vertex dummy quad.");

        for(const ShaderAttribute& attribute : m_attributes){
            int count = component_count(attribute.type);
            if(count == 0){
                ImGui::TextDisabled("%s (location %d): unsupported attribute type for preview", attribute.name.c_str(), attribute.location);
                continue;
            }

            if(!ImGui::TreeNode(attribute.name.c_str()))
                continue;

            auto& values = m_attributeValues[attribute.name];
            bool changed = false;

            for(int i = 0; i < 4; i++){
                std::string label = "Vertex " + std::to_string(i);
                ImGui::PushID(i);

                switch(count){
                    case 1: changed |= ImGui::DragFloat(label.c_str(), values[i].data(), 0.01f); break;
                    case 2: changed |= ImGui::DragFloat2(label.c_str(), values[i].data(), 0.01f); break;
                    case 3: changed |= ImGui::DragFloat3(label.c_str(), values[i].data(), 0.01f); break;
                    case 4: changed |= ImGui::DragFloat4(label.c_str(), values[i].data(), 0.01f); break;
                    default: break;
                }

                ImGui::PopID();
            }

            if(changed)
                upload_attribute(attribute);

            ImGui::TreePop();
        }
    }

    void ShaderEditorPanelSystem::draw_uniform_editors(){
        FieldContext ctx{m_app.gameScene, m_app.assets, m_app.selection, m_app};

        if(m_uniforms.empty())
            ImGui::TextDisabled("No active uniforms.");

        for(const ShaderUniform& uniform : m_uniforms){
            ImGui::PushID(uniform.name.c_str());

            switch(uniform.type){
                case ShaderDataType::Mat2:
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4:
                    ImGui::TextDisabled("%s: identity (auto)", uniform.name.c_str());
                    break;

                case ShaderDataType::Sampler2D:
                    draw_resource_field(ctx, uniform.name, m_samplerValues[uniform.name]);
                    break;

                default: {
                    int count = component_count(uniform.type);
                    if(count == 0){
                        ImGui::TextDisabled("%s: unsupported uniform type", uniform.name.c_str());
                        break;
                    }

                    MockValue& v = m_uniformValues[uniform.name];

                    if(uniform.type == ShaderDataType::Bool){
                        bool b = v[0] != 0.f;
                        if(ImGui::Checkbox(uniform.name.c_str(), &b))
                            v[0] = b ? 1.f : 0.f;
                    } else if(uniform.type == ShaderDataType::Float || uniform.type == ShaderDataType::Vec2 ||
                              uniform.type == ShaderDataType::Vec3 || uniform.type == ShaderDataType::Vec4){
                        switch(count){
                            case 1: ImGui::DragFloat(uniform.name.c_str(), v.data(), 0.01f); break;
                            case 2: ImGui::DragFloat2(uniform.name.c_str(), v.data(), 0.01f); break;
                            case 3: ImGui::DragFloat3(uniform.name.c_str(), v.data(), 0.01f); break;
                            case 4: ImGui::DragFloat4(uniform.name.c_str(), v.data(), 0.01f); break;
                            default: break;
                        }
                    } else {
                        // Int/IVec*/UInt/UVec* - edited as ints, stored back as floats.
                        int temp[4] = { static_cast<int>(v[0]), static_cast<int>(v[1]), static_cast<int>(v[2]), static_cast<int>(v[3]) };
                        bool changed = false;

                        switch(count){
                            case 1: changed = ImGui::DragInt(uniform.name.c_str(), temp); break;
                            case 2: changed = ImGui::DragInt2(uniform.name.c_str(), temp); break;
                            case 3: changed = ImGui::DragInt3(uniform.name.c_str(), temp); break;
                            case 4: changed = ImGui::DragInt4(uniform.name.c_str(), temp); break;
                            default: break;
                        }

                        if(changed)
                            for(int c = 0; c < count; c++)
                                v[c] = static_cast<float>(temp[c]);
                    }

                    break;
                }
            }

            ImGui::PopID();
        }
    }
}
