#include "draft/editor/field_widgets/audio_widgets.hpp"
#include "imgui.h"

namespace Draft {
    bool draw_component_widget(FieldContext& ctx, std::string_view label, Sound& sound){
        bool changed = false;
        ImGui::PushID(label.data(), label.data() + label.size());

        Resource<SoundBuffer> buffer = sound.get_buffer();
        if(draw_resource_field(ctx, "Buffer", buffer)){ sound.set_buffer(buffer); changed = true; }

        if(sound.is_playing()){
            if(!sound.is_paused()){
                if(ImGui::Button("Pause")){ sound.pause(); changed = true; }
            } else {
                if(ImGui::Button("Unpause")){ sound.play(); changed = true; }
            }
            ImGui::SameLine();
            if(ImGui::Button("Stop")){ sound.stop(); changed = true; }
        } else {
            if(ImGui::Button("Play")){ sound.play(); changed = true; }
        }

        bool looping = sound.get_loop();
        if(ImGui::Checkbox("Loop", &looping)){ sound.set_loop(looping); changed = true; }

        float pitch = sound.get_pitch();
        if(ImGui::DragFloat("Pitch", &pitch, 0.01f, 0.f)){ sound.set_pitch(pitch); changed = true; }

        float volume = sound.get_volume();
        if(ImGui::DragFloat("Volume", &volume, 0.1f, 0.0f, 100.f)){ sound.set_volume(volume); changed = true; }

        float minDistance = sound.get_min_distance();
        if(ImGui::DragFloat("Minimum Distance", &minDistance, 0.01f, 0.f)){ sound.set_min_distance(minDistance); changed = true; }

        float attenuation = sound.get_attenuation();
        if(ImGui::DragFloat("Attenuation", &attenuation, 0.01f, 0.f)){ sound.set_attenuation(attenuation); changed = true; }

        Vector3f position = sound.get_position();
        if(ImGui::DragFloat3("Position", &position.x, 0.1f)){ sound.set_position(position); changed = true; }
        ImGui::SameLine();
        bool relative = sound.is_relative();
        if(ImGui::Checkbox("Relative", &relative)){ sound.set_relative(relative); changed = true; }

        ImGui::PopID();
        return changed;
    }

    bool draw_component_widget(FieldContext& ctx, std::string_view label, Music& sound){
        bool changed = false;
        ImGui::PushID(label.data(), label.data() + label.size());

        if(sound.is_playing()){
            if(!sound.is_paused()){
                if(ImGui::Button("Pause")){ sound.pause(); changed = true; }
            } else {
                if(ImGui::Button("Unpause")){ sound.play(); changed = true; }
            }
            ImGui::SameLine();
            if(ImGui::Button("Stop")){ sound.stop(); changed = true; }
        } else {
            if(ImGui::Button("Play")){ sound.play(); changed = true; }
        }

        bool looping = sound.get_loop();
        if(ImGui::Checkbox("Loop", &looping)){ sound.set_loop(looping); changed = true; }

        float pitch = sound.get_pitch();
        if(ImGui::DragFloat("Pitch", &pitch, 0.01f, 0.f)){ sound.set_pitch(pitch); changed = true; }

        float volume = sound.get_volume();
        if(ImGui::DragFloat("Volume", &volume, 0.1f, 0.0f, 100.f)){ sound.set_volume(volume); changed = true; }

        float minDistance = sound.get_min_distance();
        if(ImGui::DragFloat("Minimum Distance", &minDistance, 0.01f, 0.f)){ sound.set_min_distance(minDistance); changed = true; }

        float attenuation = sound.get_attenuation();
        if(ImGui::DragFloat("Attenuation", &attenuation, 0.01f, 0.f)){ sound.set_attenuation(attenuation); changed = true; }

        Vector3f position = sound.get_position();
        if(ImGui::DragFloat3("Position", &position.x, 0.1f)){ sound.set_position(position); changed = true; }
        ImGui::SameLine();
        bool relative = sound.is_relative();
        if(ImGui::Checkbox("Relative", &relative)){ sound.set_relative(relative); changed = true; }

        ImGui::PopID();
        return changed;
    }

    bool draw_component_widget(FieldContext& ctx, std::string_view label, Resource<Music>& value){
        bool changed = draw_resource_field(ctx, label, value);

        if(value.is_valid())
            changed |= draw_component_widget(ctx, label, *value);

        return changed;
    }
}
