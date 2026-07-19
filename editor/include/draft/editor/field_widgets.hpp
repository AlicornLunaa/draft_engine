#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/asset/resource.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/serializer.hpp"

#include "imgui.h"

#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <vector>

namespace Draft {
    class EditorSelection;

    /**
     * @brief Everything a field widget might need beyond the value it's editing: the scene to
     * browse for an Entity-field picker, the assets manager to resolve a Resource<T>'s key, the
     * selection so a picked Entity field can be jumped to.
     */
    struct FieldContext {
        Scene& scene;
        AssetManager& assets;
        EditorSelection& selection;
    };

    /**
     * @brief Customization point: a type that needs bespoke rendering - a dropdown, live setter
     * calls, anything plain reflection can't express. Defines a free function
     * `bool draw_component_widget(FieldContext&, std::string_view label, T& value)` in namespace
     *
     * Found by ADL so declaring it is enough, no separate registration call needed as long as
     * its header is #include-d somewhere reachable before draw_field<T>() is instantiated for
     * that T (see field_widgets.cpp, which includes every such header once).
     */
    template<typename T>
    concept HasComponentWidget = requires(FieldContext& ctx, std::string_view label, T& value) {
        { draw_component_widget(ctx, label, value) } -> std::convertible_to<bool>;
    };

    template<typename T> struct IsResource : std::false_type {};
    template<typename T> struct IsResource<Resource<T>> : std::true_type { using AssetType = T; };

    std::string entity_label(Entity entity);
    bool draw_entity_field(FieldContext& ctx, std::string_view label, Entity& value);
    bool draw_entity_list_field(FieldContext& ctx, std::string_view label, std::vector<Entity>& values);
    bool draw_string_field(std::string_view label, std::string& value);
    bool draw_resource_field(FieldContext& ctx, std::string_view label, Resource<Texture>& value);

    /**
     * @brief Generic recursive JSON tree editor.
     * @return True if @p json was edited this frame.
     */
    bool draw_json_editor(std::string_view label, JSON& json);

    template<typename T>
    bool draw_field(FieldContext& ctx, std::string_view label, T& value);

    /**
     * @brief Fallback for anything draw_field<T>() doesn't otherwise recognize: a generic,
     * editable JSON tree (see draw_json_editor()) built from the value's own serialization and
     * written back through its own deserialization on change.
     */
    template<typename T>
    bool draw_json_fallback_field(std::string_view label, T& value){
        JSON json;
        Serializer::serialize(value, json);

        if(draw_json_editor(label, json)){
            Serializer::deserialize(value, json);
            return true;
        }

        return false;
    }

    template<typename T>
    bool draw_field(FieldContext& ctx, std::string_view label, T& value){
        std::string labelId(label);

        if constexpr(HasComponentWidget<T>){
            return draw_component_widget(ctx, label, value);
        } else if constexpr(std::same_as<T, float>){
            return ImGui::DragFloat(labelId.c_str(), &value, 0.1f);
        } else if constexpr(std::same_as<T, bool>){
            return ImGui::Checkbox(labelId.c_str(), &value);
        } else if constexpr(std::same_as<T, int>){
            return ImGui::DragInt(labelId.c_str(), &value);
        } else if constexpr(std::same_as<T, unsigned int>){
            int temp = static_cast<int>(value);
            if(ImGui::DragInt(labelId.c_str(), &temp, 1.f, 0)){
                value = static_cast<unsigned int>(temp < 0 ? 0 : temp);
                return true;
            }
            return false;
        } else if constexpr(std::same_as<T, std::string>){
            return draw_string_field(label, value);
        } else if constexpr(std::same_as<T, Vector2f>){
            return ImGui::DragFloat2(labelId.c_str(), &value.x, 0.1f);
        } else if constexpr(std::same_as<T, Vector3f>){
            return ImGui::DragFloat3(labelId.c_str(), &value.x, 0.1f);
        } else if constexpr(std::same_as<T, Entity>){
            return draw_entity_field(ctx, label, value);
        } else if constexpr(std::same_as<T, std::vector<Entity>>){
            return draw_entity_list_field(ctx, label, value);
        } else if constexpr(std::same_as<T, Resource<Texture>>){
            return draw_resource_field(ctx, label, value);
        } else if constexpr(IsResource<T>::value){
            auto key = ctx.assets.template key_for<typename IsResource<T>::AssetType>(value);
            ImGui::TextDisabled("%.*s", static_cast<int>(label.size()), label.data());
            ImGui::SameLine();
            ImGui::Text("%s", key ? key->c_str() : "(unassigned)");
            return false;
        } else if constexpr(std::is_enum_v<T>){
            if constexpr(requires(T v){ { enum_name(v) } -> std::convertible_to<std::string_view>; enum_values(v); }){
                bool changed = false;
                std::string_view current = enum_name(value);

                if(ImGui::BeginCombo(labelId.c_str(), std::string(current).c_str())){
                    for(T candidate : enum_values(value)){
                        bool isSelected = candidate == value;
                        std::string candidateName(enum_name(candidate));

                        if(ImGui::Selectable(candidateName.c_str(), isSelected)){
                            value = candidate;
                            changed = true;
                        }
                    }

                    ImGui::EndCombo();
                }

                return changed;
            } else {
                // No hand-written enum_name()/enum_values() for T yet (see body_type.hpp for the
                // pattern) should still editable as a raw underlying int rather than silently invisible.
                using U = std::underlying_type_t<T>;
                int temp = static_cast<int>(value);
                if(ImGui::DragInt(labelId.c_str(), &temp)){
                    value = static_cast<T>(static_cast<U>(temp));
                    return true;
                }
                return false;
            }
        } else if constexpr(Reflectable<T>){
            bool changed = false;
            ImGui::Indent();
            for_each_field(value, [&](std::string_view name, auto& field){
                changed |= draw_field(ctx, name, field);
            });
            ImGui::Unindent();
            return changed;
        } else {
            return draw_json_fallback_field(label, value);
        }
    }

    /**
     * @brief Bridges Runtime's type-erased ComponentTypeInterface::visit_fields() callback into
     * draw_field<T>() for exactly the field types that appear as a direct field of a
     * registered component. Anything not listed here, including a type only a future
     * game-specific component introduces, falls through to a read-only JSON dump sourced from
     * @p fallbackJson instead of crashing or vanishing.
     */
    bool draw_typeerased_field(FieldContext& ctx, std::string_view name, std::type_index type, void* valuePtr, const JSON& fallbackJson);
}
