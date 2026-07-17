#include "draft/rendering/animation.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/rendering/texture.hpp"
#include "nlohmann/json.hpp"

#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace Draft {
    namespace {
        // Draft::JSON (nlohmann::json) backs objects with std::map, so iterating a Hash-form
        // "frames" object would visit keys in lexicographic order ("frame 10" before "frame
        // 2") instead of the animation's actual sequence. ordered_json backs objects with a
        // vector instead, preserving the order they appeared in the file.
        using OrderedJSON = nlohmann::ordered_json;

        AnimationDirection parse_direction(const std::string& value){
            if(value == "reverse") return AnimationDirection::Reverse;
            if(value == "pingpong") return AnimationDirection::PingPong;
            if(value == "pingpong_reverse") return AnimationDirection::PingPongReverse;
            return AnimationDirection::Forward;
        }

        AnimationFrame parse_frame(const std::string& name, const OrderedJSON& data){
            AnimationFrame frame;
            frame.name = name;

            const OrderedJSON& bounds = data["frame"];
            frame.frame = { bounds["x"], bounds["y"], bounds["w"], bounds["h"] };

            frame.rotated = data.value("rotated", false);
            frame.trimmed = data.value("trimmed", false);

            if(data.contains("spriteSourceSize")){
                const OrderedJSON& sss = data["spriteSourceSize"];
                frame.spriteSourceSize = { sss["x"], sss["y"], sss["w"], sss["h"] };
            }

            if(data.contains("sourceSize")){
                const OrderedJSON& size = data["sourceSize"];
                frame.sourceSize = { size["w"].get<float>(), size["h"].get<float>() };
            }

            frame.duration = data.value("duration", 0.f);
            frame.userData = data.value("data", std::string());

            return frame;
        }
    }

    // Constructor
    Animation::Animation(const FileHandle& handle, AssetManager& assets) {
        OrderedJSON data = OrderedJSON::parse(handle.read_string());

        // Frames can be exported as either a name-keyed object ("Hash") or an array ("Array"), each element carrying its own "filename".
        const OrderedJSON& framesNode = data["frames"];
        if(framesNode.is_object()){
            for(auto it = framesNode.begin(); it != framesNode.end(); ++it)
                frames.push_back(parse_frame(it.key(), it.value()));
        } else {
            for(const auto& entry : framesNode)
                frames.push_back(parse_frame(entry.value("filename", std::string()), entry));
        }

        for(const auto& frame : frames){
            totalFrameTime += frame.duration;
            frameEndTimes.push_back(totalFrameTime);
        }

        const OrderedJSON& meta = data["meta"];
        app = meta.value("app", std::string());
        version = meta.value("version", std::string());
        imageFilename = meta.value("image", std::string());
        format = meta.value("format", std::string());

        // Aseprite exports "scale" as a string (e.g. "1"), not a number
        if(meta.contains("scale"))
            scale = std::stof(meta["scale"].get<std::string>());

        if(meta.contains("size")){
            const OrderedJSON& size = meta["size"];
            sheetSize = { size["w"].get<unsigned int>(), size["h"].get<unsigned int>() };
        }

        if(meta.contains("frameTags")){
            for(const auto& entry : meta["frameTags"]){
                AnimationTag tag;
                tag.name = entry.value("name", std::string());
                tag.from = entry.value("from", 0);
                tag.to = entry.value("to", 0);
                tag.direction = parse_direction(entry.value("direction", std::string("forward")));
                tag.userData = entry.value("data", std::string());

                if(entry.contains("color"))
                    tag.color = Color(entry["color"].get<std::string>());

                build_tag_playback(tag);
                tags.emplace(tag.name, std::move(tag));
            }
        }

        if(meta.contains("layers")){
            for(const auto& entry : meta["layers"]){
                AnimationLayer layer;
                layer.name = entry.value("name", std::string());
                layer.opacity = entry.value("opacity", 255.f);
                layer.blendMode = entry.value("blendMode", std::string());
                layer.userData = entry.value("data", std::string());

                layers.push_back(std::move(layer));
            }
        }

        if(meta.contains("slices")){
            for(const auto& entry : meta["slices"]){
                AnimationSlice slice;
                slice.name = entry.value("name", std::string());
                slice.userData = entry.value("data", std::string());

                if(entry.contains("color"))
                    slice.color = Color(entry["color"].get<std::string>());

                if(entry.contains("keys")){
                    for(const auto& keyEntry : entry["keys"]){
                        AnimationSliceKey key;
                        key.frame = keyEntry.value("frame", 0);

                        const OrderedJSON& bounds = keyEntry["bounds"];
                        key.bounds = {
                            bounds["x"].get<int>(), bounds["y"].get<int>(),
                            bounds["w"].get<int>(), bounds["h"].get<int>()
                        };

                        if(keyEntry.contains("center")){
                            const OrderedJSON& center = keyEntry["center"];
                            key.center = IntRect{
                                center["x"].get<int>(), center["y"].get<int>(),
                                center["w"].get<int>(), center["h"].get<int>()
                            };
                        }

                        if(keyEntry.contains("pivot")){
                            const OrderedJSON& pivot = keyEntry["pivot"];
                            key.pivot = Vector2i{ pivot["x"].get<int>(), pivot["y"].get<int>() };
                        }

                        slice.keys.push_back(std::move(key));
                    }
                }

                slices.push_back(std::move(slice));
            }
        }

        std::filesystem::path imagePath = std::filesystem::path(handle.get_path()).parent_path() / imageFilename;
        texture = assets.get<Texture>(imagePath.generic_string());
    }

    // Private functions
    void Animation::build_tag_playback(AnimationTag& tag){
        if(frames.empty() || tag.from < 0 || tag.to < 0 || static_cast<size_t>(tag.from) >= frames.size() || static_cast<size_t>(tag.to) >= frames.size())
            return;

        std::vector<size_t> forward;
        for(int i = tag.from; i <= tag.to; i++)
            forward.push_back(static_cast<size_t>(i));

        switch(tag.direction){
            case AnimationDirection::Forward:
                tag.playbackFrames = forward;
                break;

            case AnimationDirection::Reverse:
                tag.playbackFrames.assign(forward.rbegin(), forward.rend());
                break;

            case AnimationDirection::PingPong:
                tag.playbackFrames = forward;
                for(int i = static_cast<int>(forward.size()) - 2; i > 0; i--)
                    tag.playbackFrames.push_back(forward[i]);
                break;

            case AnimationDirection::PingPongReverse:
                tag.playbackFrames.assign(forward.rbegin(), forward.rend());
                for(int i = 1; i < static_cast<int>(forward.size()) - 1; i++)
                    tag.playbackFrames.push_back(forward[i]);
                break;
        }

        for(size_t index : tag.playbackFrames){
            tag.totalPlaybackTime += frames[index].duration;
            tag.playbackCumulativeTime.push_back(tag.totalPlaybackTime);
        }
    }

    TextureRegion Animation::make_region(size_t frameIndex) const {
        const AnimationFrame& frame = frames[frameIndex];
        float textureHeight = texture->get_properties().size.y;

        return {
            texture,
            FloatRect{
                frame.frame.x,
                textureHeight - frame.frame.y - frame.frame.height,
                frame.frame.width,
                frame.frame.height
            }
        };
    }

    // Functions
    TextureRegion Animation::get_frame(float frameTime) const {
        if(frames.empty())
            throw std::runtime_error("Animation::get_frame(): animation has no frames");

        // Keep time
        if(totalFrameTime > 0.f)
            frameTime = fmodf(frameTime, totalFrameTime);

        // Get required frame
        for(size_t i = 0; i < frames.size(); i++){
            if(frameEndTimes[i] > frameTime){
                return make_region(i);
            }
        }

        // Defensive fallback for floating-point rounding leaving frameTime a hair past the last frame's own end time
        return make_region(frames.size() - 1);
    }

    TextureRegion Animation::get_frame(const std::string& tagName, float frameTime) const {
        auto it = tags.find(tagName);
        if(it == tags.end())
            throw std::runtime_error("Animation::get_frame(): unknown tag \"" + tagName + "\"");

        const AnimationTag& tag = it->second;
        if(tag.playbackFrames.empty())
            throw std::runtime_error("Animation::get_frame(): tag \"" + tagName + "\" has no frames");

        if(tag.totalPlaybackTime > 0.f)
            frameTime = fmodf(frameTime, tag.totalPlaybackTime);

        for(size_t i = 0; i < tag.playbackFrames.size(); i++){
            if(tag.playbackCumulativeTime[i] > frameTime){
                return make_region(tag.playbackFrames[i]);
            }
        }

        return make_region(tag.playbackFrames.back());
    }

    const AnimationTag& Animation::get_tag(const std::string& name) const {
        auto it = tags.find(name);
        if(it == tags.end())
            throw std::runtime_error("Animation::get_tag(): unknown tag \"" + name + "\"");

        return it->second;
    }
}
