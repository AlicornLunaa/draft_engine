#pragma once

#include "draft/build_tools/asset_pipeline.hpp"

namespace Draft {
    /**
     * @brief ImGui drag-drop payload type name for an asset browser entry of this AssetKind, so a
     * drop target (e.g. a Resource<Texture> field) only accepts a matching kind. Shared between
     * AssetBrowserPanelSystem (the drag source) and field_widgets.hpp (the drop targets).
     */
    const char* asset_drag_payload_type(AssetKind kind);
}
