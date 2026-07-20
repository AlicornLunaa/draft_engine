#pragma once

#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Editor-only preferences, persisted to EditorProject::manifest_path() under an
     * "editor" key alongside that same file's other top-level keys ("module", "assetsDir", ...)
     * without touching this shape.
     */
    struct EditorSettings {
        float positionSnapStep = 10.f;
        float rotationSnapStepDegrees = 15.f;

        DRAFT_REFLECTABLE(EditorSettings, positionSnapStep, rotationSnapStepDegrees)
    };
}
