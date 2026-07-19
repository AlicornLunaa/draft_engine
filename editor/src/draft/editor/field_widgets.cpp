#include "draft/editor/field_widgets.hpp"
#include "draft/components/tag_component.hpp"
#include "draft/editor/selection.hpp"
#include "draft/physics/body_type.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/audio/music.hpp"

#include "imgui.h"

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

    bool draw_resource_field(FieldContext& ctx, std::string_view label, Resource<Texture>& value){
        auto key = ctx.assets.key_for<Texture>(value);

        if(value.is_valid()){
            ImTextureID texId = static_cast<ImTextureID>(static_cast<intptr_t>(value->get_texture_handle()));
            ImGui::Image(texId, ImVec2(32, 32));
            ImGui::SameLine();
        }

        ImGui::TextDisabled("%.*s", static_cast<int>(label.size()), label.data());
        ImGui::SameLine();
        ImGui::Text("%s", key ? key->c_str() : "(unassigned)");
        return false;
    }

    bool draw_typeerased_field(FieldContext& ctx, std::string_view name, std::type_index type, void* valuePtr, const JSON& fallbackJson){
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
        if(type == typeid(TextureRegion)) return draw_field(ctx, name, *static_cast<TextureRegion*>(valuePtr));
        if(type == typeid(Resource<Texture>)) return draw_field(ctx, name, *static_cast<Resource<Texture>*>(valuePtr));
        if(type == typeid(Resource<Animation>)) return draw_field(ctx, name, *static_cast<Resource<Animation>*>(valuePtr));
        if(type == typeid(Resource<Music>)) return draw_field(ctx, name, *static_cast<Resource<Music>*>(valuePtr));

        // No live-editable widget registered for this field's type, show it read-only from the
        // component's own JSON serialization instead of leaving it silently invisible.
        std::string key(name);
        ImGui::TextDisabled("%s:", key.c_str());
        ImGui::SameLine();

        if(fallbackJson.contains(key))
            ImGui::TextWrapped("%s", fallbackJson.at(key).dump(2).c_str());
        else
            ImGui::TextWrapped("<unavailable>");

        return false;
    }
}
