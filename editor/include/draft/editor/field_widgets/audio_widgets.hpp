#pragma once

#include "draft/audio/music.hpp"
#include "draft/audio/sound.hpp"
#include "draft/editor/field_widgets.hpp"

namespace Draft {
    /**
     * @brief Custom inspector widget for Sound. Includes an asset picker (drag-drop + "..."
     * popup) for the underlying SoundBuffer, same as a plain Resource<Texture> field gets.
     */
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Sound& sound);

    /**
     * @brief Custom inspector widget for Music.
     */
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Music& music);

    /**
     * @brief Custom inspector widget for MusicComponent::track. Draws the same asset picker
     * every other Resource<T> field gets (via draw_resource_field())
     */
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Resource<Music>& value);
}
