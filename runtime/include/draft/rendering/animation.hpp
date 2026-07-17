#pragma once

#include "draft/asset/resource.hpp"
#include "draft/math/color.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/util/files/file_handle.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Draft {
    class AssetManager;

    /**
     * @brief Playback direction of a frame tag, as exported by Aseprite.
     */
    enum class AnimationDirection {
        Forward,
        Reverse,
        PingPong,
        PingPongReverse
    };

    /**
     * @brief One entry from an Aseprite JSON export's "frames" list/map. `frame` and
     * `spriteSourceSize` are in the exporter's top-down (y-down) pixel space, unlike the
     * bottom-up TextureRegion Animation::get_frame() returns.
     */
    struct AnimationFrame {
        std::string name;
        FloatRect frame;
        bool rotated = false;
        bool trimmed = false;
        FloatRect spriteSourceSize;
        Vector2f sourceSize;
        float duration = 0.f;
        std::string userData;
    };

    /**
     * @brief A named, directional sub-range of an animation's frames (Aseprite's "frameTags").
     */
    struct AnimationTag {
        std::string name;
        int from = 0;
        int to = 0;
        AnimationDirection direction = AnimationDirection::Forward;
        Color color;
        std::string userData;

        // Precomputed playback order and cumulative timing along it,
        // so Animation::get_frame(tag, time) doesn't rebuild this on every call.
        std::vector<size_t> playbackFrames;
        std::vector<float> playbackCumulativeTime;
        float totalPlaybackTime = 0.f;
    };

    /**
     * @brief One key from an Aseprite slice's "keys" list, active from its frame index onward.
     */
    struct AnimationSliceKey {
        int frame = 0;
        IntRect bounds;
        std::optional<IntRect> center;
        std::optional<Vector2i> pivot;
    };

    /**
     * @brief A named region tracked across frames (Aseprite's "slices").
     */
    struct AnimationSlice {
        std::string name;
        Color color;
        std::string userData;
        std::vector<AnimationSliceKey> keys;
    };

    /**
     * @brief One entry from an Aseprite export's "layers" list.
     */
    struct AnimationLayer {
        std::string name;
        float opacity = 255.f;
        std::string blendMode;
        std::string userData;
    };

    /**
     * @brief A sprite-sheet animation loaded from a full Aseprite JSON export. Every frame's
     * region/duration/trim info, frame tags, slices, and layers. The spritesheet texture named
     * by the export's meta.image is resolved relative to the JSON file's own directory and
     * loaded through @p assets.
     */
    class Animation {
    private:
        // Variables
        Resource<Texture> texture;
        std::vector<AnimationFrame> frames;
        std::unordered_map<std::string, AnimationTag> tags;
        std::vector<AnimationSlice> slices;
        std::vector<AnimationLayer> layers;

        std::string app;
        std::string version;
        std::string imageFilename;
        std::string format;
        Vector2u sheetSize;
        float scale = 1.f;

        std::vector<float> frameEndTimes;
        float totalFrameTime = 0.f;

        // Private functions
        void build_tag_playback(AnimationTag& tag);
        TextureRegion make_region(size_t frameIndex) const;

    public:
        // Constructors
        Animation(const FileHandle& handle, AssetManager& assets);
        Animation(const Animation& other) = delete;
        Animation(Animation&& other) noexcept = default;

        // Operators
        Animation& operator=(const Animation& other) = delete;
        Animation& operator=(Animation&& other) noexcept = default;

        // Functions
        /**
         * @brief Gets the frame covering @p frameTime, wrapping around the animation's total duration.
         * @throws std::runtime_error if this animation has no frames.
         */
        TextureRegion get_frame(float frameTime = 0.f) const;

        /**
         * @brief Gets the frame covering @p frameTime within @p tagName's range, wrapping
         * around that tag's own duration and following its playback direction.
         * @throws std::runtime_error if @p tagName isn't a known tag, or that tag has no frames.
         */
        TextureRegion get_frame(const std::string& tagName, float frameTime) const;

        bool has_tag(const std::string& name) const { return tags.contains(name); }

        /**
         * @throws std::runtime_error if @p name isn't a known tag.
         */
        const AnimationTag& get_tag(const std::string& name) const;

        const std::vector<AnimationFrame>& get_frames() const { return frames; }
        const std::unordered_map<std::string, AnimationTag>& get_tags() const { return tags; }
        const std::vector<AnimationSlice>& get_slices() const { return slices; }
        const std::vector<AnimationLayer>& get_layers() const { return layers; }

        const std::string& get_app() const { return app; }
        const std::string& get_version() const { return version; }
        const std::string& get_image_filename() const { return imageFilename; }
        const std::string& get_format() const { return format; }
        Vector2u get_sheet_size() const { return sheetSize; }
        float get_scale() const { return scale; }
        float get_total_frame_time() const { return totalFrameTime; }
        Resource<Texture> get_texture() const { return texture; }
    };
}
