#pragma once

#include "RmlUi/Core/Context.h"

class SystemInterface_GLFW;
class RenderInterface_GL3;

namespace Draft {
    class RenderWindow;

    namespace UI {
        class RMLBackend {
        public:
            // Statics
            static int s_backendCount;
            static SystemInterface_GLFW* s_systemInterface;
            static RenderInterface_GL3* s_renderInterface;

            // Variables
            RenderWindow& m_windowRef;
            Rml::Context* m_context = nullptr;

        public:
            // Constructors
            RMLBackend(RenderWindow& window);
            ~RMLBackend();

            // Operators


            // Functions
        };
    }
}