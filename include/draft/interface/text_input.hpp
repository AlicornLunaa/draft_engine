#pragma once

#include "draft/interface/panel.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include <string>

namespace Draft {
    class TextInput : public Panel {
    private:
        // Variables
        TextRenderer textRenderer;
        std::string* str = nullptr;
        bool selected = false;

        Vector2f textBounds;
        float animTimer = 0.f;

    public:
        // Constructor
        TextInput(float x, float y, float w, float h, std::string* str, Panel* parent = nullptr);

        // Functions
        virtual bool handle_event(const Event& event) override;
        virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
    };
};