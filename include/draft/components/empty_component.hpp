#pragma once

namespace Draft {
    /**
     * @brief Example component for how components work within the registry
     */
    struct EmptyComponent {
        EmptyComponent(const EmptyComponent&) = default;
        EmptyComponent() {}

        // operator sf::Drawable& (){ return *sprite; }
        // operator sf::Sprite& (){ return *sprite; }
        // operator const sf::Sprite& (){ return *sprite; }
    };
}