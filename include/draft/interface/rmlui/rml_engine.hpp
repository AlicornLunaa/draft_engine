#pragma once

class SystemInterface_GLFW;
class RenderInterface_GL3;

namespace Draft {
    /// Forward decls
    class RenderWindow;

    /// Manages an RML instance
    class RmlEngine {
    public:
        // Statics
        static int s_backendCount;
        static SystemInterface_GLFW* s_systemInterface;
        static RenderInterface_GL3* s_renderInterface;

        // Variables
        RenderWindow& m_windowRef;

    public:
        // Constructors
        RmlEngine(RenderWindow& window);
        RmlEngine(const RmlEngine& other) = delete;
        RmlEngine(RmlEngine&& other);
        ~RmlEngine();

        // Operators
        RmlEngine& operator=(const RmlEngine& other) = delete;
        RmlEngine& operator=(RmlEngine&& other) = delete;

        // Functions
        void start_frame() const;
        void end_frame() const;
    };
}