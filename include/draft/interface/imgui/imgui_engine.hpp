#pragma once

#include "imgui.h"

namespace Draft {
    /// Forward decl
    class RenderWindow;

    /// Handles setting up ImGUI and related callbacks, its a class for automatic destruction
    class ImguiEngine {
    private:
        // Variables
        ImGuiContext* ctx = nullptr;
    
    public:
        // Constructors
        ImguiEngine(RenderWindow& window);
        ImguiEngine(const ImguiEngine& other) = delete;
        ImguiEngine(ImguiEngine&& other);
        ~ImguiEngine();

        // Operators
        ImguiEngine& operator=(const ImguiEngine& other) = delete;
        ImguiEngine& operator=(ImguiEngine&& other);

        // Functions
        void start_frame() const;
        void end_frame() const;
    };
};