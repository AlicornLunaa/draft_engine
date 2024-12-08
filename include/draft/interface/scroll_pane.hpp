#pragma once

#include "draft/interface/panel.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
namespace Draft {
    class ScrollPane : public Panel {
    private:
        // Variables
        Vector2f mousePosition{};
        FloatRect handleBounds{};
        bool grabbing = false; // Used for click & drag

    public:
        // Public variables
        float scroll = 0.f;

        // Constructors
        ScrollPane(float x, float y, float w, float h, Panel* parent = nullptr);

        // Functions
        virtual bool handle_event(const Event& event) override;
        virtual void paint(const Time& deltaTime, SpriteBatch& batch) override;
    };
};