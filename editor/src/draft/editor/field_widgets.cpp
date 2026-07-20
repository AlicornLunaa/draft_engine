#include "draft/editor/field_widgets.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/editor/editor_application.hpp"
#include "draft/editor/field_widgets/camera_widget.hpp" // IWYU pragma: keep
#include "draft/editor/field_widgets/collider_widget.hpp" // IWYU pragma: keep
#include "draft/editor/project.hpp"
#include "draft/editor/selection.hpp"
#include "draft/physics/body_type.hpp"
#include "draft/physics/collider.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/audio/music.hpp"

#include "imgui.h"

#include <array>

namespace Draft {
    std::string entity_label(Entity entity){
        // TODO: move this into some entity helper namespace
        if(!entity.is_valid())
            return "(none)";

        if(auto* tag = entity.try_get_component<TagComponent>(); tag && !tag->tag.empty())
            return tag->tag;

        return "Entity " + std::to_string(static_cast<uint32_t>(static_cast<entt::entity>(entity)));
    }

    bool draw_entity_field(FieldContext& ctx, std::string_view label, Entity& value){
        bool changed = false;
        std::string labelId(label);
        std::string current = entity_label(value);

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
        if(ImGui::BeginCombo(labelId.c_str(), current.c_str())){
            if(ImGui::Selectable("(none)", !value.is_valid())){
                value = Entity();
                changed = true;
            }

            for(entt::entity raw : ctx.scene.get_registry().storage<entt::entity>()){
                Entity candidate(&ctx.scene, raw);
                if(!candidate.is_valid())
                    continue;

                bool isSelected = candidate == value;
                if(ImGui::Selectable(entity_label(candidate).c_str(), isSelected)){
                    value = candidate;
                    changed = true;
                }
            }

            ImGui::EndCombo();
        }

        if(value.is_valid()){
            ImGui::SameLine();
            if(ImGui::SmallButton("Select"))
                ctx.selection.set(value);
        }

        return changed;
    }

    bool draw_entity_list_field(FieldContext& ctx, std::string_view label, std::vector<Entity>& values){
        ImGui::TextDisabled("%.*s", static_cast<int>(label.size()), label.data());
        ImGui::Indent();

        bool changed = false;
        int removeIndex = -1;

        for(size_t i = 0; i < values.size(); i++){
            ImGui::PushID(static_cast<int>(i));

            Entity value = values[i];
            if(draw_entity_field(ctx, "##entity", value)){
                values[i] = value;
                changed = true;
            }

            ImGui::SameLine();
            if(ImGui::SmallButton("Remove"))
                removeIndex = static_cast<int>(i);

            ImGui::PopID();
        }

        if(removeIndex >= 0){
            values.erase(values.begin() + removeIndex);
            changed = true;
        }

        if(ImGui::SmallButton("Add Entity")){
            values.push_back(Entity());
            changed = true;
        }

        ImGui::Unindent();
        return changed;
    }

    bool draw_string_field(std::string_view label, std::string& value){
        std::array<char, 256> buffer{};
        std::strncpy(buffer.data(), value.c_str(), buffer.size() - 1);

        std::string labelId(label);
        if(ImGui::InputText(labelId.c_str(), buffer.data(), buffer.size())){
            value = buffer.data();
            return true;
        }

        return false;
    }

    bool draw_asset_picker_list(FieldContext& ctx, AssetKind kind, std::string& outSelectedKey){
        static std::array<char, 256> s_searchBuffer{};

        if(ImGui::IsWindowAppearing())
            s_searchBuffer[0] = '\0';

        ImGui::SetNextItemWidth(300.f);
        ImGui::InputTextWithHint("##AssetPickerSearch", "Search...", s_searchBuffer.data(), s_searchBuffer.size());

        if(!ctx.app.has_project())
            return false;

        bool picked = false;

        ImGui::BeginChild("##AssetPickerList", ImVec2(300.f, 200.f));
        for(const AssetTask& task : collect_project_assets(ctx.app.project()->root())){
            if(task.kind != kind)
                continue;

            if(s_searchBuffer[0] != '\0' && task.key.find(s_searchBuffer.data()) == std::string::npos)
                continue;

            if(ImGui::Selectable(task.key.c_str())){
                outSelectedKey = task.key;
                picked = true;
            }
        }
        ImGui::EndChild();

        if(picked)
            ImGui::CloseCurrentPopup();

        return picked;
    }

    bool draw_json_editor(std::string_view label, JSON& json){
        std::string labelId(label);
        ImGui::PushID(labelId.c_str());
        bool changed = false;

        if(json.is_boolean()){
            bool value = json.get<bool>();
            if(ImGui::Checkbox(labelId.c_str(), &value)){ json = value; changed = true; }
        } else if(json.is_number_float()){
            float value = json.get<float>();
            if(ImGui::DragFloat(labelId.c_str(), &value, 0.1f)){ json = value; changed = true; }
        } else if(json.is_number_integer()){
            int value = json.get<int>();
            if(ImGui::DragInt(labelId.c_str(), &value)){ json = value; changed = true; }
        } else if(json.is_string()){
            std::string value = json.get<std::string>();
            if(draw_string_field(label, value)){ json = value; changed = true; }
        } else if(json.is_object() || json.is_array()){
            ImGui::TextDisabled("%s", labelId.c_str());
            ImGui::Indent();

            for(auto element : json.items())
                changed |= draw_json_editor(element.key(), static_cast<JSON&>(element.value()));

            ImGui::Unindent();
        } else {
            ImGui::TextDisabled("%s: null", labelId.c_str());
        }

        ImGui::PopID();
        return changed;
    }

    bool draw_typeerased_field(FieldContext& ctx, std::string_view name, std::type_index type, void* valuePtr, JSON& componentJson, bool& usedJsonFallback){
        if(type == typeid(float)) return draw_field(ctx, name, *static_cast<float*>(valuePtr));
        if(type == typeid(bool)) return draw_field(ctx, name, *static_cast<bool*>(valuePtr));
        if(type == typeid(int)) return draw_field(ctx, name, *static_cast<int*>(valuePtr));
        if(type == typeid(unsigned int)) return draw_field(ctx, name, *static_cast<unsigned int*>(valuePtr));
        if(type == typeid(std::string)) return draw_field(ctx, name, *static_cast<std::string*>(valuePtr));
        if(type == typeid(Vector2f)) return draw_field(ctx, name, *static_cast<Vector2f*>(valuePtr));
        if(type == typeid(Vector3f)) return draw_field(ctx, name, *static_cast<Vector3f*>(valuePtr));
        if(type == typeid(Entity)) return draw_field(ctx, name, *static_cast<Entity*>(valuePtr));
        if(type == typeid(std::vector<Entity>)) return draw_field(ctx, name, *static_cast<std::vector<Entity>*>(valuePtr));
        if(type == typeid(BodyType)) return draw_field(ctx, name, *static_cast<BodyType*>(valuePtr));
        if(type == typeid(Camera)) return draw_field(ctx, name, *static_cast<Camera*>(valuePtr));
        if(type == typeid(Collider)) return draw_field(ctx, name, *static_cast<Collider*>(valuePtr));
        if(type == typeid(TextureRegion)) return draw_field(ctx, name, *static_cast<TextureRegion*>(valuePtr));
        if(type == typeid(Resource<Texture>)) return draw_field(ctx, name, *static_cast<Resource<Texture>*>(valuePtr));
        if(type == typeid(Resource<Animation>)) return draw_field(ctx, name, *static_cast<Resource<Animation>*>(valuePtr));
        if(type == typeid(Resource<Music>)) return draw_field(ctx, name, *static_cast<Resource<Music>*>(valuePtr));

        // No typed pointer recovery for this field's type so it can't be edited in place like the cases above.
        usedJsonFallback = true;

        std::string key(name);
        if(!componentJson.contains(key))
            componentJson[key] = JSON();

        return draw_json_editor(name, componentJson[key]);
    }
}
