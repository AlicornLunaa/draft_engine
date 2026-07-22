#include "draft/editor/panels/rml_editor_panel.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/logger.hpp"

#include "RmlUi/Core/Context.h"
#include "RmlUi/Core/DataVariable.h"
#include "RmlUi/Core/Variant.h"

#include "imgui.h"

#include <algorithm>
#include <regex>

namespace Draft {
    namespace {
        // Binds a JSON tree to RmlUi's generic Rml::VariableDefinition extension point
        class JsonVariableDefinition;

        JsonVariableDefinition& scalar_definition();
        JsonVariableDefinition& array_definition();
        JsonVariableDefinition& struct_definition();

        JsonVariableDefinition& definition_for(const nlohmann::json& node){
            if(node.is_object())
                return struct_definition();
            if(node.is_array())
                return array_definition();
            return scalar_definition();
        }

        class JsonVariableDefinition final : public Rml::VariableDefinition {
        public:
            explicit JsonVariableDefinition(Rml::DataVariableType type) : Rml::VariableDefinition(type) {}

            bool Get(void* ptr, Rml::Variant& variant) override {
                const nlohmann::json& node = *static_cast<const nlohmann::json*>(ptr);

                if(node.is_boolean())
                    variant = node.get<bool>();
                else if(node.is_number_float())
                    variant = node.get<double>();
                else if(node.is_number())
                    variant = node.get<int>();
                else if(node.is_string())
                    variant = node.get<std::string>();
                else
                    return false; // object/array/null, not directly representable as a scalar

                return true;
            }

            bool Set(void* ptr, const Rml::Variant& variant) override {
                nlohmann::json& node = *static_cast<nlohmann::json*>(ptr);

                if(node.is_boolean()){
                    bool value;
                    if(!variant.GetInto<bool>(value)) return false;
                    node = value;
                } else if(node.is_number_float()){
                    double value;
                    if(!variant.GetInto<double>(value)) return false;
                    node = value;
                } else if(node.is_number()){
                    int value;
                    if(!variant.GetInto<int>(value)) return false;
                    node = value;
                } else if(node.is_string()){
                    std::string value;
                    if(!variant.GetInto<std::string>(value)) return false;
                    node = value;
                } else {
                    return false;
                }

                return true;
            }

            int Size(void* ptr) override {
                const nlohmann::json& node = *static_cast<const nlohmann::json*>(ptr);
                return node.is_array() ? static_cast<int>(node.size()) : 0;
            }

            Rml::DataVariable Child(void* ptr, const Rml::DataAddressEntry& address) override {
                nlohmann::json& node = *static_cast<nlohmann::json*>(ptr);

                if(node.is_array()){
                    if(address.index < 0){
                        if(address.name == "size")
                            return Rml::MakeLiteralIntVariable(static_cast<int>(node.size()));
                        return Rml::DataVariable();
                    }

                    if(address.index >= static_cast<int>(node.size()))
                        return Rml::DataVariable();

                    nlohmann::json& child = node.at(static_cast<size_t>(address.index));
                    return Rml::DataVariable(&definition_for(child), &child);
                }

                if(node.is_object()){
                    auto it = node.find(address.name);
                    if(it == node.end())
                        return Rml::DataVariable();

                    nlohmann::json& child = it.value();
                    return Rml::DataVariable(&definition_for(child), &child);
                }

                return Rml::DataVariable();
            }
        };

        JsonVariableDefinition& scalar_definition(){ static JsonVariableDefinition instance(Rml::DataVariableType::Scalar); return instance; }
        JsonVariableDefinition& array_definition(){ static JsonVariableDefinition instance(Rml::DataVariableType::Array); return instance; }
        JsonVariableDefinition& struct_definition(){ static JsonVariableDefinition instance(Rml::DataVariableType::Struct); return instance; }

        // Minimal ImGui::InputTextMultiline(std::string*)
        struct ResizeCallbackData { std::string* str; };

        int resize_callback(ImGuiInputTextCallbackData* data){
            if(data->EventFlag == ImGuiInputTextFlags_CallbackResize){
                auto* userData = static_cast<ResizeCallbackData*>(data->UserData);
                userData->str->resize(static_cast<size_t>(data->BufTextLen));
                data->Buf = userData->str->data();
            }

            return 0;
        }

        bool input_text_multiline(const char* label, std::string& str, const ImVec2& size){
            ResizeCallbackData userData{&str};
            return ImGui::InputTextMultiline(label, str.data(), str.capacity() + 1, size, ImGuiInputTextFlags_CallbackResize, resize_callback, &userData);
        }
    }

    RmlEditorPanelSystem::RmlEditorPanelSystem(EditorApplication& app)
        : m_app(app), m_rmlUi(Vector2u(800, 600)), m_previewTarget(FramebufferProperties{.size = {800, 600}})
    {
        m_context = &m_rmlUi.add_context("rml_editor_preview", Vector2i(800, 600));
        sync_data_model_text();
    }

    void RmlEditorPanelSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default || !m_app.rmlEditorPanelVisible)
            return;

        std::string title = "Rml Editor";
        if(m_app.rmlEditorAssetKey)
            title += " - " + std::filesystem::path(*m_app.rmlEditorAssetKey).filename().string();
        title += "###RmlEditor";

        ImGui::SetNextWindowSize({520, 720}, ImGuiCond_FirstUseEver);

        if(ImGui::Begin(title.c_str(), &m_app.rmlEditorPanelVisible)){
            if(!m_app.rmlEditorAssetKey){
                ImGui::TextDisabled("Double click an RML document in the Asset Browser to preview it.");
            } else {
                if(m_loadedKey != *m_app.rmlEditorAssetKey)
                    load_asset(*m_app.rmlEditorAssetKey);

                bool changed = false;
                if(m_documentWatcher && m_documentWatcher->poll())
                    changed = true;
                for(GameModuleWatcher& watcher : m_dependencyWatchers)
                    if(watcher.poll())
                        changed = true;
                if(changed)
                    reload_document();

                if(!m_loadError.empty())
                    ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_loadError.c_str());

                constexpr float dataHeight = 300.f;
                float previewHeight = std::max(ImGui::GetContentRegionAvail().y - dataHeight - ImGui::GetStyle().ItemSpacing.y, 100.f);

                draw_preview(previewHeight);
                ImGui::Separator();

                ImGui::BeginChild("RmlEditorData");
                if(ImGui::CollapsingHeader("Canvas", ImGuiTreeNodeFlags_DefaultOpen))
                    draw_canvas_settings();
                if(ImGui::CollapsingHeader("Data Model", ImGuiTreeNodeFlags_DefaultOpen))
                    draw_data_model_editor();

                ImGui::Separator();
                if(ImGui::Button("Save Mock Data"))
                    save_mock_data();
                ImGui::EndChild();
            }
        }

        ImGui::End();
    }

    void RmlEditorPanelSystem::load_asset(const std::string& key){
        m_loadedKey = key;
        m_loadError.clear();
        m_documentWatcher.reset();
        m_dependencyWatchers.clear();
        m_dataModel = JSON::object();
        m_dataModelParseError.clear();

        if(!m_app.has_project())
            return;

        m_documentWatcher.emplace(m_app.project()->root() / key);

        load_mock_data(); // also resyncs m_dataModelText
        reload_document(); // unloads whatever was previously shown, loads `key`, rebuilds watchers + data model
    }

    void RmlEditorPanelSystem::reload_document(){
        Rml::Context* context = m_context->get_context();

        if(m_document){
            context->UnloadDocument(m_document);
            m_document = nullptr;
        }

        m_loadError.clear();

        if(m_app.has_project()){
            std::filesystem::path fullPath = m_app.project()->root() / m_loadedKey;
            FileHandle handle = HostFileSystem().open(fullPath);

            if(!handle.exists()){
                m_loadError = "File not found: " + fullPath.string();
            } else {
                try {
                    m_document = m_context->load_document(handle);

                    if(m_document)
                        m_document->Show();
                    else
                        m_loadError = "RmlUi failed to parse this document (see log).";
                } catch(const std::exception& e){
                    m_loadError = e.what();
                }
            }
        }

        rebuild_watchers();
        rebuild_data_model();
    }

    void RmlEditorPanelSystem::rebuild_watchers(){
        m_dependencyWatchers.clear();

        if(!m_app.has_project())
            return;

        std::filesystem::path fullPath = m_app.project()->root() / m_loadedKey;
        for(const std::filesystem::path& dependency : discover_rcss_links(fullPath))
            m_dependencyWatchers.emplace_back(dependency);
    }

    std::vector<std::filesystem::path> RmlEditorPanelSystem::discover_rcss_links(const std::filesystem::path& rmlPath) const {
        std::vector<std::filesystem::path> links;

        FileHandle handle = HostFileSystem().open(rmlPath);
        if(!handle.exists())
            return links;

        std::string text = handle.read_string();

        // RmlUi doesn't expose a loaded document's resolved stylesheet source paths back out, so
        // dependency discovery has to re-scan the raw markup for <link type="text/rcss" href="...">
        // tags. One level only, RCSS @import chains inside a linked stylesheet aren't followed.
        static const std::regex linkTag(R"(<link\b[^>]*>)", std::regex::icase);
        static const std::regex typeAttr(R"(type\s*=\s*["']text/rcss["'])", std::regex::icase);
        static const std::regex hrefAttr(R"(href\s*=\s*["']([^"']+)["'])", std::regex::icase);

        for(auto it = std::sregex_iterator(text.begin(), text.end(), linkTag); it != std::sregex_iterator(); ++it){
            std::string tag = it->str();
            if(!std::regex_search(tag, typeAttr))
                continue;

            std::smatch hrefMatch;
            if(std::regex_search(tag, hrefMatch, hrefAttr))
                links.push_back(rmlPath.parent_path() / hrefMatch[1].str());
        }

        return links;
    }

    void RmlEditorPanelSystem::rebuild_data_model(){
        Rml::Context* context = m_context->get_context();
        context->RemoveDataModel("mock_data");
        m_dataModelHandle = Rml::DataModelHandle();

        if(!m_dataModel.is_object() || m_dataModel.empty())
            return;

        Rml::DataModelConstructor constructor = context->CreateDataModel("mock_data", nullptr, true);

        // Each top-level key becomes one bound {{variable}} and JsonVariableDefinition::Child() recurses into whatever nesting that key's value has from there.
        for(auto it = m_dataModel.begin(); it != m_dataModel.end(); ++it){
            nlohmann::json& child = it.value();
            constructor.BindCustomDataVariable(it.key(), Rml::DataVariable(&definition_for(child), &child));
        }

        m_dataModelHandle = constructor.GetModelHandle();
    }

    void RmlEditorPanelSystem::apply_data_model_text(){
        try {
            JSON parsed = JSON::parse(m_dataModelText);

            if(!parsed.is_object()){
                m_dataModelParseError = "Root must be a JSON object (each top-level key becomes one bound variable).";
                return;
            }

            m_dataModel = std::move(parsed);
            m_dataModelParseError.clear();
            rebuild_data_model();
        } catch(const std::exception& e){
            // Last-good m_dataModel/bindings are left untouched
            m_dataModelParseError = e.what();
        }
    }

    void RmlEditorPanelSystem::sync_data_model_text(){
        m_dataModelText = m_dataModel.dump(4);
    }

    std::filesystem::path RmlEditorPanelSystem::mock_data_path() const {
        std::string sanitized = m_loadedKey;
        std::replace(sanitized.begin(), sanitized.end(), '/', '_');
        std::replace(sanitized.begin(), sanitized.end(), '\\', '_');

        return m_app.project()->root() / ".draft-editor" / "rml_previews" / (sanitized + ".json");
    }

    void RmlEditorPanelSystem::load_mock_data(){
        m_dataModel = JSON::object();
        m_dataModelParseError.clear();
        m_useScreenSize = true;
        m_customCanvasSize = Vector2u(800, 600);

        FileHandle handle = HostFileSystem().open(mock_data_path());

        if(handle.exists()){
            try {
                JSON json(handle);

                if(json.contains("dataModel") && json["dataModel"].is_object())
                    m_dataModel = json["dataModel"];

                m_useScreenSize = json.value("useScreenSize", true);
                m_customCanvasSize.x = static_cast<unsigned int>(json.value("canvasWidth", 800));
                m_customCanvasSize.y = static_cast<unsigned int>(json.value("canvasHeight", 600));
            } catch(const std::exception& e){
                Logger::println(LogLevel::Warning, "RmlEditor", "Failed to load mock data for " + m_loadedKey + ": " + e.what());
            }
        }

        sync_data_model_text();
    }

    void RmlEditorPanelSystem::save_mock_data() const {
        if(!m_app.has_project())
            return;

        JSON json = JSON::object();
        json["dataModel"] = m_dataModel;
        json["useScreenSize"] = m_useScreenSize;
        json["canvasWidth"] = m_customCanvasSize.x;
        json["canvasHeight"] = m_customCanvasSize.y;

        HostFileSystem().open(mock_data_path()).write_string(json.dump(4));
    }

    void RmlEditorPanelSystem::draw_preview(float availableHeight){
        float rowWidth = ImGui::GetContentRegionAvail().x;

        Vector2u canvasSize = m_useScreenSize
            ? Math::max(m_app.gameApp.get_output().get_properties().size, Vector2u(1, 1))
            : Vector2u(std::max(m_customCanvasSize.x, 1u), std::max(m_customCanvasSize.y, 1u));

        if(m_previewTarget.get_size() != canvasSize)
            m_previewTarget.set_size(canvasSize);

        m_context->get_context()->SetDimensions(Rml::Vector2i((int)canvasSize.x, (int)canvasSize.y));

        m_previewTarget.begin();
        m_rmlUi.resize(canvasSize);
        m_rmlUi.render(Time::seconds(1.f / 60.f), RenderLayer::Overlay);
        m_previewTarget.end();

        float aspect = (float)canvasSize.x / (float)canvasSize.y;
        float imageHeight = std::clamp(availableHeight, 64.f, 2048.f);
        float imageWidth = imageHeight * aspect;

        if(imageWidth > rowWidth){
            imageWidth = rowWidth;
            imageHeight = imageWidth / aspect;
        }

        float centerOffset = (rowWidth - imageWidth) * 0.5f;
        if(centerOffset > 0.f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + centerOffset);

        ImTextureID textureId = static_cast<ImTextureID>(static_cast<intptr_t>(m_previewTarget.get_texture().get_texture_handle()));
        ImGui::Image(textureId, ImVec2(imageWidth, imageHeight), ImVec2(0, 1), ImVec2(1, 0));
    }

    void RmlEditorPanelSystem::draw_canvas_settings(){
        ImGui::Checkbox("Use Screen Size", &m_useScreenSize);

        if(m_useScreenSize){
            Vector2u current = Math::max(m_app.gameApp.get_output().get_properties().size, Vector2u(1, 1));
            ImGui::TextDisabled("Current: %u x %u", current.x, current.y);
        } else {
            int size[2] = { (int)m_customCanvasSize.x, (int)m_customCanvasSize.y };
            if(ImGui::DragInt2("Canvas Size", size, 1.f, 1, 8192)){
                m_customCanvasSize.x = static_cast<unsigned int>(std::max(size[0], 1));
                m_customCanvasSize.y = static_cast<unsigned int>(std::max(size[1], 1));
            }
        }
    }

    void RmlEditorPanelSystem::draw_data_model_editor(){
        if(input_text_multiline("##DataModelJson", m_dataModelText, ImVec2(-1, 220)))
            apply_data_model_text();

        if(!m_dataModelParseError.empty())
            ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "%s", m_dataModelParseError.c_str());
    }
}
