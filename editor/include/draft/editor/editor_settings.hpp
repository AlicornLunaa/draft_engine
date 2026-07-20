#pragma once

#include "draft/util/reflectable.hpp"

namespace Draft {
    /**
     * @brief Editor-only preferences, persisted to EditorProject::manifest_path() under an
     * "editor" key so a future unified manifest (module location, asset location, ...) can add
     * its own keys alongside this one without touching this shape.
     */
    struct EditorSettings {
        float positionSnapStep = 10.f;
        float rotationSnapStepDegrees = 15.f;

        DRAFT_REFLECTABLE(EditorSettings, positionSnapStep, rotationSnapStepDegrees)
    };
}
