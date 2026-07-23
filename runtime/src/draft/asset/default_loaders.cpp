#include "draft/asset/default_loaders.hpp"
#include "draft/asset/asset_manager.hpp"
#include "draft/audio/music.hpp"
#include "draft/audio/sound_buffer.hpp"
#include "draft/ecs/scene_serialization_context.hpp"
#include "draft/physics/collider.hpp"
#include "draft/rendering/animation.hpp"
#include "draft/rendering/font.hpp"
#include "draft/rendering/image.hpp"
#include "draft/rendering/model.hpp"
#include "draft/rendering/particle_system.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/rendering/texture.hpp"
#include "draft/rendering/texture_packer.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"
#include "draft/util/localization.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/serialization/context.hpp"
#include "draft/util/serialization/resource_serializer.hpp" // IWYU pragma: keep
#include "draft/util/serialization/serializer.hpp"
#include <any>
#include <stdexcept>

namespace Draft {
    namespace Loaders {
        template<>
        void register_default_loader<Animation>(AssetManager& assets){
            // Animation resolves its own spritesheet texture from the JSON's meta.image field,
            // so (like Model) there's no separable off-thread stage.
            assets.register_loader<Animation>(
                [](const FileHandle& handle, AssetManager& assets){
                    return Animation(handle, assets);
                }
            );
        }

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
            // Parsing text into a JSON tree doesn't touch AssetManager, so it can run off-thread.
            // Turning that JSON into a ParticleProps needs to resolve texture/shader Resource
            // fields through AssetManager though, so that part has to run on the finish stage.
            assets.register_loader<ParticleProps>(
                [](const FileHandle& handle){
                    std::string text = handle.read_string();
                    return text.empty() ? JSON::object() : JSON::parse(text);
                },
                [](std::any data, AssetManager& assets){
                    JSON json = std::any_cast<JSON>(data);

                    SceneSerializationContext ctx;
                    ctx.assets = &assets;
                    Serializer::ScopedContext<SceneSerializationContext> scope(ctx);

                    // A freshly created .particle file may be empty or only specify a few
                    // fields, so fields missing from the JSON keep ParticleProps's own defaults
                    // instead of throwing.
                    ParticleProps props;
                    for_each_field(props, [&](std::string_view name, auto& field){
                        std::string key(name);
                        if(json.contains(key))
                            Serializer::deserialize(field, json.at(key));
                    });

                    return props;
                }
            );
        }

        template<>
        void register_default_loader<Shader>(AssetManager& assets){
            // Shader(FileHandle) already does its own GL upload internally, so (like Model)
            // there's no separable off-thread stage.
            assets.register_loader<Shader>(
                [](const FileHandle& handle, AssetManager&){
                    return Shader(handle);
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
        void register_default_loader<Music>(AssetManager& assets){
            assets.register_loader<Music>(
                [](const FileHandle& handle){
                    return handle;
                },
                [](std::any data, AssetManager&){
                    return Music(std::any_cast<FileHandle>(data));
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

        template<>
        void register_default_loader<Localization>(AssetManager& assets){
            assets.register_loader<Localization>(
                [](const FileHandle& handle){
                    if(!handle.is_directory()){
                        throw std::runtime_error("Localization loader takes a directory.");
                    }

                    Localization localization;

                    for(auto& file : handle.list()){
                        if(file.is_directory()) continue;

                        try {
                            localization.load_language(file);
                        } catch(...){
                            Logger::println(LogLevel::Warning, "Localization", "Failed to load language pack " + file.stem());
                            continue;
                        }
                    }

                    return localization;
                },
                [](std::any data, AssetManager&){
                    return std::any_cast<Localization>(data);
                }
            );
        }
    }
}