#pragma once

namespace Clydesdale::ECS {
    struct EmptyComponent {
        EmptyComponent(const EmptyComponent&) = default;
        EmptyComponent() {}

        // operator sf::Drawable& (){ return *sprite; }
        // operator sf::Sprite& (){ return *sprite; }
        // operator const sf::Sprite& (){ return *sprite; }
    };
}