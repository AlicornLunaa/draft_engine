#include "draft/editor/asset_drag_drop.hpp"

namespace Draft {
    const char* asset_drag_payload_type(AssetKind kind){
        switch(kind){
            case AssetKind::Texture: return "ASSET_TEXTURE";
            case AssetKind::Font: return "ASSET_FONT";
            case AssetKind::Model: return "ASSET_MODEL";
            case AssetKind::Sound: return "ASSET_SOUND";
            case AssetKind::Music: return "ASSET_MUSIC";
            case AssetKind::Scene: return "ASSET_SCENE";
            case AssetKind::Prefab: return "ASSET_PREFAB";
            case AssetKind::Animation: return "ASSET_ANIMATION";
            case AssetKind::Language: return "ASSET_LANGUAGE";
            case AssetKind::Particle: return "ASSET_PARTICLE";
            case AssetKind::RML: return "ASSET_RML";
            case AssetKind::RCSS: return "ASSET_RCSS";
            default: return "ASSET_UNKNOWN";
        }
    }
}
