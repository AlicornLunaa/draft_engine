#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/asset/resource.hpp"
#include "draft/build_tools/asset_pipeline.hpp"
#include "draft/ecs/entity.hpp"
#include "draft/ecs/scene.hpp"
#include "draft/editor/asset_drag_drop.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/serializer.hpp"

#include "imgui.h"

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <vector>

namespace Draft {
    class EditorSelection;
    class EditorApplication;
    class Animation;

    /**
     * @brief Everything a field widget might need beyond the value it's editing: the scene to
     * browse for an Entity-field picker, the assets manager to resolve a Resource<T>'s key, the
     * selection so a picked Entity field can be jumped to, and the owning EditorApplication.
     */
    struct FieldContext {
        Scene& scene;
        AssetManager& assets;
        EditorSelection& selection;
        EditorApplication& app;
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

    template<typename T> struct IsOptionalResource : std::false_type {};
    template<typename T> struct IsOptionalResource<std::optional<Resource<T>>> : std::true_type { using AssetType = T; };

    /**
     * @brief Maps an asset type T to the AssetKind an asset browser entry needs to have for a
     * Resource<T> field to accept it as a drag-drop drop. Only specialized for types that both
     * have a working AssetManager default loader and already appear as a Resource<T> field
     * somewhere - unspecialized T just means "no drop target", not an error.
     */
    template<typename T> struct AssetKindOf { static constexpr bool has_kind = false; };
    template<> struct AssetKindOf<Texture> { static constexpr bool has_kind = true; static constexpr AssetKind kind = AssetKind::Texture; };
    template<> struct AssetKindOf<Animation> { static constexpr bool has_kind = true; static constexpr AssetKind kind = AssetKind::Animation; };

    std::string entity_label(Entity entity);
    bool draw_entity_field(FieldContext& ctx, std::string_view label, Entity& value);
    bool draw_entity_list_field(FieldContext& ctx, std::string_view label, std::vector<Entity>& values);
    bool draw_string_field(std::string_view label, std::string& value);

    /**
     * @brief Draws a search box + scrollable list of every project asset of @p kind (must be
     * called with a popup already begun, e.g. via ImGui::BeginPopup()). Shared across every
     * Resource<T> field's picker button, T-agnostic since it only ever hands back a key string.
     * @return True if an entry was picked this frame (@p outSelectedKey set, popup closed).
     */
    bool draw_asset_picker_list(FieldContext& ctx, AssetKind kind, std::string& outSelectedKey);

    /**
     * @brief Shows a Resource<T>'s current key (plus a thumbnail for Texture). If T has an
     * AssetKindOf, also offers two ways to reassign it: a drop target accepting a matching-kind
     * drag from the asset browser, and a "..." button opening draw_asset_picker_list().
     */
    template<typename T>
    bool draw_resource_field(FieldContext& ctx, std::string_view label, Resource<T>& value){
        auto key = ctx.assets.template key_for<T>(value);

        if constexpr(std::same_as<T, Texture>){
            if(value.is_valid()){
                ImTextureID texId = static_cast<ImTextureID>(static_cast<intptr_t>(value->get_texture_handle()));
                ImGui::Image(texId, ImVec2(32, 32));
                ImGui::SameLine();
            }
        }

        ImGui::TextDisabled("%.*s", static_cast<int>(label.size()), label.data());
        ImGui::SameLine();
        ImGui::Text("%s", key ? key->c_str() : "(unassigned)");

        bool changed = false;

        if constexpr(AssetKindOf<T>::has_kind){
            if(ImGui::BeginDragDropTarget()){
                if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(asset_drag_payload_type(AssetKindOf<T>::kind))){
                    std::string droppedKey(static_cast<const char*>(payload->Data), static_cast<size_t>(payload->DataSize));
                    value = ctx.assets.template get<T>(droppedKey);
                    changed = true;
                }

                ImGui::EndDragDropTarget();
            }

            std::string pickerId = "PickAsset##" + std::string(label);
            ImGui::SameLine();
            if(ImGui::SmallButton(("...##" + std::string(label)).c_str()))
                ImGui::OpenPopup(pickerId.c_str());

            if(ImGui::BeginPopup(pickerId.c_str())){
                std::string pickedKey;
                if(draw_asset_picker_list(ctx, AssetKindOf<T>::kind, pickedKey)){
                    value = ctx.assets.template get<T>(pickedKey);
                    changed = true;
                }

                ImGui::EndPopup();
            }
        }

        return changed;
    }

    /**
     * @brief Same as draw_resource_field(), for a field where "unassigned" is itself a
     * meaningful value (e.g. ParticleProps::texture, where nullopt means "use the renderer's
     * own fallback") rather than just the not-yet-loaded state a plain Resource<T> starts in.
     * Adds a Clear button to explicitly get back to nullopt, since draw_resource_field() itself
     * only ever assigns a value, never clears one.
     */
    template<typename T>
    bool draw_optional_resource_field(FieldContext& ctx, std::string_view label, std::optional<Resource<T>>& value){
        Resource<T> current = value.value_or(Resource<T>());
        bool changed = draw_resource_field(ctx, label, current);

        if(changed)
            value = current;

        if(value){
            ImGui::SameLine();
            if(ImGui::SmallButton(("Clear##" + std::string(label)).c_str())){
                value.reset();
                changed = true;
            }
        }

        return changed;
    }

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
        } else if constexpr(std::same_as<T, Vector4f>){
            return ImGui::ColorEdit4(labelId.c_str(), &value.x);
        } else if constexpr(std::same_as<T, Entity>){
            return draw_entity_field(ctx, label, value);
        } else if constexpr(std::same_as<T, std::vector<Entity>>){
            return draw_entity_list_field(ctx, label, value);
        } else if constexpr(IsResource<T>::value){
            return draw_resource_field(ctx, label, value);
        } else if constexpr(IsOptionalResource<T>::value){
            return draw_optional_resource_field(ctx, label, value);
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
     * game-specific component introduces, falls through to a generic, editable JSON subtree of
     * @p componentJson (see draw_json_editor()) instead of vanishing. That edit isn't written
     * back through this field alone (there's no typed pointer to deserialize into for an
     * unrecognized type). The caller is expected to re-serialize + deserialize the *whole*
     * component through ComponentTypeInterface (same virtual entry point load_scene() uses) when
     * this returns true, exactly the way InspectorPanelSystem does.
     * @param usedJsonFallback Set to true if this call went through the JSON subtree (so the
     * caller knows it needs the re-serialize/overlay/deserialize write-back), left untouched
     * (caller should default it false) if a typed widget handled and already live-applied the edit itself.
     * @return True if the field was edited this frame, whether through a typed widget or the
     * generic JSON subtree.
     */
    bool draw_typeerased_field(FieldContext& ctx, std::string_view name, std::type_index type, void* valuePtr, JSON& componentJson, bool& usedJsonFallback);
}
