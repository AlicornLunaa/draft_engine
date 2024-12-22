#include "draft/interface/clay/clay_interface.hpp"
#include "draft/math/glm.hpp"
#include "clay.h"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/font.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"

#include <string>
#include <cstdlib>

namespace Draft {
    namespace UI {
        // Data to help bridge static to object oriented
        static Clay* activeClayInterface = nullptr;
        static SpriteBatch* activeBatch = nullptr;

        // Private functions
        Clay_Dimensions Clay::measure_text(Clay_String* text, Clay_TextElementConfig* config){
            // Get text bounds from the text renderer and font
            assert(activeClayInterface && "Cant measure text before a batch was started");

            Clay* that = activeClayInterface;
            Font* font = that->fonts[config->fontId];
            font->set_font_size(config->fontSize);

            TextProperties textProps;
            textProps.str = std::string(text->chars, text->length);
            textProps.font = font;
            textProps.scale = 1;

            auto res = that->textBatch.get_text_bounds(textProps);
            return {res.x, res.y};
        }

        // Constructors
        Clay::Clay(Application& app) : appRef(app), camera({{0, 0, 10}, {0, 0, -1}, 0, 1280, 0, 720, 0.1f, 100.f}) {
            Vector2u size = app.window.get_size();
            camera = {{0, 0, 10}, {0, 0, -1}, 0, (float)size.x, 0, (float)size.y, 0.1f, 100.f};
            load_font(Assets::manager.get<Font>("assets/fonts/default.ttf", true));

            totalMemorySize = Clay_MinMemorySize();
            arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, std::malloc(totalMemorySize));
            Clay_Initialize(arena, { (float)size.x, (float)size.y });
            Clay_SetMeasureTextFunction(measure_text);
        }

        Clay::~Clay(){
            std::free(arena.memory);
        }

        // Functions
        uint Clay::load_font(const Resource<Font>& font){
            fonts.push_back(font);
            return fonts.size() - 1;
        }

        void Clay::begin(SpriteBatch& batch, const Time& dt){
            assert(!activeClayInterface && !activeBatch && "Must end previous batch before beginning a new one");
            activeClayInterface = this;
            activeBatch = &batch;

            Vector2f size = appRef.window.get_size();
            camera = OrthoCamera(
                camera.get_position(),
                camera.get_forward(),
                0,
                size.x,
                0,
                size.y,
                camera.get_near(),
                camera.get_far()
            );
            batch.set_proj_matrix(camera.get_combined());
            batch.set_trans_matrix(Matrix4(1.f));
            batch.set_shader(uiShader);

            Vector2f pointer = appRef.mouse.get_position();
            Vector2f delta = appRef.mouse.get_scroll();
            Clay_SetLayoutDimensions({size.x, size.y});
            Clay_SetPointerState((Clay_Vector2){ pointer.x, pointer.y }, appRef.mouse.is_pressed(Mouse::LEFT_BUTTON));
            Clay_UpdateScrollContainers(true, (Clay_Vector2){ delta.x, delta.y }, dt.as_seconds());

            Clay_BeginLayout();
        }

        void Clay::end(){
            assert(activeClayInterface && activeBatch && "Must begin a batch before you can end it");
            assert(activeClayInterface == this && "Cannot end a random batch");

            Clay_RenderCommandArray renderCommands = Clay_EndLayout();
            SpriteBatch& batch = *activeBatch;
            Vector2f winSize = appRef.window.get_size();

            for(int i = 0; i < renderCommands.length; i++){
                Clay_RenderCommand* renderCommand = &renderCommands.internalArray[i];

                switch(renderCommand->commandType){
                case CLAY_RENDER_COMMAND_TYPE_NONE:
                    break;

                case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                    auto& bounds = renderCommand->boundingBox;
                    auto& col = renderCommand->config.rectangleElementConfig->color;

                    batch.draw({
                        nullptr, {},
                        {bounds.x, winSize.y - bounds.y},
                        0.f,
                        {bounds.width, bounds.height},
                        {0, bounds.height},
                        0.f,
                        {col.r / 255.f, col.g / 255.f, col.b / 255.f, col.a / 255.f}
                    });

                    break;
                }

                case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                    auto& bounds = renderCommand->boundingBox;
                    auto* conf = renderCommand->config.textElementConfig;

                    TextProperties props;
                    props.color = {conf->textColor.r / 255.f, conf->textColor.g / 255.f, conf->textColor.b / 255.f, conf->textColor.a / 255.f};
                    props.font = fonts[conf->fontId];
                    props.font->set_font_size(conf->fontSize);
                    props.position = {bounds.x, winSize.y - bounds.y};
                    props.origin = {0, 1};
                    props.scale = 1.f;
                    props.str = std::string(renderCommand->text.chars, renderCommand->text.length);

                    textBatch.draw_text(batch, props);
                    batch.set_shader(uiShader);

                    break;
                }

                case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START:
                    scissor.box = { renderCommand->boundingBox.x, renderCommand->boundingBox.y, renderCommand->boundingBox.width, renderCommand->boundingBox.height };
                    scissor.begin();
                    break;

                case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END:
                    scissor.end();
                    break;

                case CLAY_RENDER_COMMAND_TYPE_BORDER:
                case CLAY_RENDER_COMMAND_TYPE_IMAGE:
                case CLAY_RENDER_COMMAND_TYPE_CUSTOM:
                    break;
                }
                batch.flush();
            }

            activeClayInterface = nullptr;
            activeBatch = nullptr;
        }
    };
};