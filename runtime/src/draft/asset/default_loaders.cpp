#include "draft/asset/default_loaders.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/physics/collider.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/texture_packer.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include <any>
#include <utility>

namespace Draft {
    namespace Loaders {
        template<>
        void register_default_loader<Collider>(AssetManager& assets){
            assets.register_loader<Collider>(
                [](const FileHandle& handle){
                    Collider collider;
                    Collider::deserialize(collider, JSON::parse(handle.read_string()));
                    return collider;
                },
                [](std::any data, AssetManager&){
                    auto collider = std::any_cast<Collider>(data);
                    return collider;
                }
            );
        }

        template<>
        void register_default_loader<Font>(AssetManager& assets){
            assets.register_loader<Font>(
                [](const FileHandle& handle){
                    return handle.read_bytes();
                },
                [](std::any data, AssetManager&){
                    auto fontData = std::any_cast<Binary::ByteArray>(data);
                    return Font(fontData);
                }
            );
        }

        template<>
        void register_default_loader<Image>(AssetManager& assets){
            assets.register_loader<Image>(
                [](const FileHandle& handle){
                    return Image(handle);
                },
                [](std::any data, AssetManager&){
                    return std::any_cast<Image>(data);
                }
            );
        }

        template<>
        void register_default_loader<JSON>(AssetManager& assets){
            assets.register_loader<JSON>(
                [](const FileHandle& handle){
                    return JSON::parse(handle.read_string());
                },
                [](std::any data, AssetManager&){
                    return std::any_cast<JSON>(data);
                }
            );
        }

        template<>
        void register_default_loader<Model>(AssetManager& assets){
            // Model(FileHandle) already does its own GL upload internally (embedded textures,
            // vertex buffers, ...), so there's no separable off-thread stage
            assets.register_loader<Model>(
                [](const FileHandle& handle, AssetManager&){
                    return Model(handle);
                }
            );
        }

        template<>
        void register_default_loader<TexturePacker>(AssetManager& assets){
            assets.register_loader<TexturePacker>(
                [](const FileHandle& handle){
                    TexturePacker packer;

                    HostFileSystem fs;
                    std::vector<FileHandle> handles;
                    for(std::filesystem::recursive_directory_iterator i(handle.get_path()), end; i != end; i++){
                        if(!i->is_directory() && (i->path().extension() == ".png" || i->path().extension() == ".jpeg" || i->path().extension() == ".jpg")){
                            handles.push_back(fs.open(i->path()));
                        }
                    }
                    packer.pack(handles);

                    return packer;
                },
                [](std::any data, AssetManager&){
                    auto packer = std::any_cast<TexturePacker>(data);
                    packer.create();
                    return packer;
                }
            );
        }

        template<>
        void register_default_loader<ParticleProps>(AssetManager& assets){
            assets.register_loader<ParticleProps>(
                [](const FileHandle& handle){
                    JSON data = JSON::parse(handle.read_string());
                    
                    ParticleProps props;
                    props.velocity.x = data["velocity"]["x"];
                    props.velocity.y = data["velocity"]["y"];
                    props.velocityVariation.x = data["velocity_variation"]["x"];
                    props.velocityVariation.y = data["velocity_variation"]["y"];
                    props.colorBegin.r = data["color_begin"]["r"];
                    props.colorBegin.g = data["color_begin"]["g"];
                    props.colorBegin.b = data["color_begin"]["b"];
                    props.colorBegin.a = data["color_begin"]["a"];
                    props.colorEnd.r = data["color_end"]["r"];
                    props.colorEnd.g = data["color_end"]["g"];
                    props.colorEnd.b = data["color_end"]["b"];
                    props.colorEnd.a = data["color_end"]["a"];
                    props.sizeBegin = data["size_begin"];
                    props.sizeEnd = data["size_end"];
                    props.sizeVariation = data["size_variation"];
                    props.lifeTime = data["lifetime"];

                    return std::make_pair(props, data["texture"]);
                },
                [](std::any data, AssetManager& assets){
                    auto [props, texturePath] = std::any_cast<std::pair<ParticleProps, std::string>>(data);
                    props.texture = assets.get<Texture>(texturePath);
                    return props;
                }
            );
        }

        template<>
        void register_default_loader<SoundBuffer>(AssetManager& assets){
            assets.register_loader<SoundBuffer>(
                [](const FileHandle& handle){
                    return handle.read_bytes();
                },
                [](std::any data, AssetManager&){
                    return SoundBuffer(std::any_cast<Binary::ByteArray>(data));
                }
            );
        }

        template<>
        void register_default_loader<Texture>(AssetManager& assets){
            assets.register_loader<Texture>(
                [](const FileHandle& handle){
                    return Image(handle);
                },
                [](std::any data, AssetManager&){
                    return Texture(std::any_cast<Image>(data));
                }
            );
        }
    }
}