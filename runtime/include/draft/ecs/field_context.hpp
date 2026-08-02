#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/ecs/scene.hpp"

namespace Draft {
    class EditorSelection;
    class EditorApplication;

    /**
     * @brief Everything a field widget might need beyond the value it's editing
     */
    struct FieldContext {
        Scene& scene;
        AssetManager& assets;
        EditorSelection& selection;
        EditorApplication& app;
    };
}
