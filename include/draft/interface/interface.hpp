#pragma once

#include "draft/core/scene.hpp"
#include "draft/interface/unit_value.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/color.hpp"
#include "glm/common.hpp"

#include <cmath>
#include <vector>

namespace Draft {
    namespace UI {
        // Styling
        struct Edges {
            UnitValue left = UnitValue::Auto;
            UnitValue top = UnitValue::Auto;
            UnitValue right = UnitValue::Auto;
            UnitValue bottom = UnitValue::Auto;
        };

        struct Size {
            UnitValue width = UnitValue::Auto;
            UnitValue minWidth = 0_pixels;
            UnitValue maxWidth = UnitValue::Auto;

            UnitValue height = UnitValue::Auto;
            UnitValue minHeight = 0_pixels;
            UnitValue maxHeight = UnitValue::Auto;
        };

        struct GlobalStyle {
            Vector2<UnitValue> position = {UnitValue::Auto, UnitValue::Auto};
            Size size;
            Edges margins = {0_pixels, 0_pixels, 0_pixels, 0_pixels};
            Edges padding = {0_pixels, 0_pixels, 0_pixels, 0_pixels};
            Color foregroundColor = Color::WHITE;
            Color backgroundColor = Color::BLACK;
            bool visible = true;
        };

        // Render information
        struct Command {
            enum Type { SPRITE, TEXT } type;

            float x;
            float y;
            Color color;

            union {
                struct {
                    const Texture* texture;
                    float width;
                    float height;
                } sprite;

                struct {
                    const char* str;
                    Font* font;
                    uint fontSize;
                } text;
            };
        };

        struct Context {
            SpriteBatch& batch;
            TextRenderer& textBatch;
            FloatRect bounds; // The container size
        };

        // Widgets
        struct Layout {
            GlobalStyle style;

            FloatRect get_content_box(const Context& ctx) const {
                // Sizes the element to get the position and size to hold it
                FloatRect box;

                // Handle different states, such as automatic sizing or automatic positioning
                Vector2f size = {
                    style.size.width.is_auto() ? 0 : style.size.width.get(ctx.bounds.width),
                    style.size.height.is_auto() ? 0 : style.size.height.get(ctx.bounds.height)
                };

                Vector2f minSize = {
                    style.size.minWidth.is_auto() ? 0 : style.size.minWidth.get(ctx.bounds.width),
                    style.size.minHeight.is_auto() ? 0 : style.size.minHeight.get(ctx.bounds.height)
                };

                Vector2f maxSize = {
                    style.size.maxWidth.is_auto() ? INFINITY : style.size.maxWidth.get(ctx.bounds.width),
                    style.size.maxHeight.is_auto() ? INFINITY : style.size.maxHeight.get(ctx.bounds.height)
                };

                Vector2f position = {
                    style.position.x.is_auto() ? 0 : style.position.x.get(ctx.bounds.width),
                    style.position.y.is_auto() ? 0 : style.position.y.get(ctx.bounds.height)
                };

                // Render this panel
                Vector4f marginsCalc = {
                    style.margins.left.get(ctx.bounds.width),
                    style.margins.top.get(ctx.bounds.width),
                    style.margins.right.get(ctx.bounds.width),
                    style.margins.bottom.get(ctx.bounds.width)
                };

                Vector4f paddingCalc = {
                    style.padding.left.get(ctx.bounds.width),
                    style.padding.top.get(ctx.bounds.width),
                    style.padding.right.get(ctx.bounds.width),
                    style.padding.bottom.get(ctx.bounds.width)
                };

                box.x = position.x + marginsCalc.x;
                box.y = position.y + marginsCalc.y;
                box.width = size.x - (marginsCalc.x - marginsCalc.z) + (paddingCalc.x + paddingCalc.z);
                box.height = size.y - (marginsCalc.y - marginsCalc.w) + (paddingCalc.y + paddingCalc.w);

                box.width = Math::clamp(box.width, minSize.x, maxSize.x);
                box.height = Math::clamp(box.height, minSize.y, maxSize.y);

                return box;
            }

            virtual void render(Context ctx, std::vector<Command>& commands) const = 0;
        };

        struct Panel : public Layout {
            // Lays children out absolutely
            std::vector<Layout*> children;

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                // Get the size and position of the inner box
                FloatRect content = get_content_box(ctx);

                Command cmd;
                cmd.type = Command::SPRITE;
                cmd.x = content.x;
                cmd.y = content.y;
                cmd.color = style.backgroundColor;
                cmd.sprite.width = content.width;
                cmd.sprite.height = content.height;
                cmd.sprite.texture = nullptr;
                commands.push_back(cmd);
                uint thisCmdIndex = commands.size() - 1;

                // Calculate children render commands
                size_t startCount = commands.size();
                Context newCtx(ctx.batch, ctx.textBatch, {0, 0, content.width, content.height});

                for(Layout* ptr : children){
                    // Obtain render commands for children
                    ptr->render(newCtx, commands);
                }

                size_t count = commands.size() - startCount;

                // With the count of objects which are the children commands, move the child to be relative to this layout object
                Vector2f topLeftPadding(style.padding.left.get(ctx.bounds.width), style.padding.top.get(ctx.bounds.width));

                for(size_t i = 0; i < count; i++){
                    Command& cmd = commands[i + startCount];
                    cmd.x += content.x + topLeftPadding.x; // Padding is needed here to push it off the side
                    cmd.y += content.y + topLeftPadding.y;
                }

                // With the children now positioned, calculate the size for auto-sizing
                Vector4f childDimensions(0, 0, INFINITY, INFINITY);

                for(size_t i = 0; i < count; i++){
                    Command& cmd = commands[i + startCount];
                    Vector2f elementDimensions;

                    if(cmd.type == Command::SPRITE){
                        elementDimensions.x = cmd.sprite.width;
                        elementDimensions.y = cmd.sprite.height;
                    } else if(cmd.type == Command::TEXT){
                        elementDimensions = ctx.textBatch.get_text_bounds({ std::string(cmd.text.str), cmd.text.font, cmd.text.fontSize });
                    }

                    childDimensions.x = Math::max(cmd.x + elementDimensions.x, childDimensions.x);
                    childDimensions.y = Math::max(cmd.y + elementDimensions.y, childDimensions.y);
                    childDimensions.z = Math::min(cmd.x, childDimensions.z);
                    childDimensions.w = Math::min(cmd.y, childDimensions.w);
                }

                // Keep constraints
                Vector2f minSize = {
                    style.size.minWidth.is_auto() ? 0 : style.size.minWidth.get(ctx.bounds.width),
                    style.size.minHeight.is_auto() ? 0 : style.size.minHeight.get(ctx.bounds.height)
                };

                Vector2f maxSize = {
                    style.size.maxWidth.is_auto() ? INFINITY : style.size.maxWidth.get(ctx.bounds.width),
                    style.size.maxHeight.is_auto() ? INFINITY : style.size.maxHeight.get(ctx.bounds.height)
                };

                ctx.bounds.width = Math::clamp(ctx.bounds.width, minSize.x, maxSize.x);
                ctx.bounds.height = Math::clamp(ctx.bounds.height, minSize.y, maxSize.y);

                // Update auto sizing if enabled to fit contents of children
                Command& thisCmd = commands[thisCmdIndex];
                if(style.size.width.is_auto() && !Math::isinf(childDimensions.z)){
                    thisCmd.sprite.width += childDimensions.x - childDimensions.z;
                }

                if(style.size.height.is_auto() && !Math::isinf(childDimensions.z)){
                    thisCmd.sprite.height += childDimensions.y - childDimensions.w;
                }
            };
        };

        struct Label : public Layout {
            std::string str = "Hello World!";
            Font* font = nullptr;
            uint fontSize = 22;

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                // Render text in the center if the given box
                Command cmd;
                cmd.type = Command::TEXT;
                cmd.x = 0;
                cmd.y = 0;
                cmd.color = style.foregroundColor;
                cmd.text.str = str.c_str();
                cmd.text.font = font;
                cmd.text.fontSize = fontSize;
                commands.push_back(cmd);
            };
        };

        struct Button : public Layout {
            Label btnLabel;

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
            };
        };

        // Main control panel for rendering
        class Interface {
        private:
            // Variables
            Scene* scene = nullptr;
            SpriteBatch batch;
            TextRenderer textBatch;
            Resource<Shader> defaultShader = Assets::manager.get<Shader>("assets/shaders/default");

            OrthoCamera camera = {{0, 0, 10}, {0, 0, -1}, 0, 1280, 720, 0};
            Context masterCtx;

        public:
            // Constructors
            Interface(Scene* scene);
            ~Interface() = default;

            // Functions
            void begin();
            void draw(const Layout& layout);
            void end();
        };
    };
};