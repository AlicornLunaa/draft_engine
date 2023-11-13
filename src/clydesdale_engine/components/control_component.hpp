#pragma once

namespace Clydesdale {
    namespace ECS {
        struct ControlComponent {
            int id = 0; // Dummy data or else EnTT wont recognize it

            ControlComponent(const ControlComponent&) = default;
            ControlComponent() {}
        };
    }
}