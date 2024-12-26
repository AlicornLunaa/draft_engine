#pragma once

#include "draft/core/scene.hpp"
#include "draft/interface/unit_value.hpp"
#include "draft/math/glm.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/util/color.hpp"

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
            Vector2<UnitValue> size = {UnitValue::Auto, UnitValue::Auto};
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

            union {
                struct {
                    const Texture* texture;
                    float width;
                    float height;
                    Color color;
                } sprite;

                struct {
                    const char* str;
                    Font* font;
                    int fontSize;
                } text;
            };
        };

        struct Context {
            SpriteBatch& batch;
            FloatRect bounds; // The container size
        };

        // Widgets
        struct Layout {
            GlobalStyle style;
            virtual void render(Context ctx, std::vector<Command>& commands) const = 0;
        };

        struct Panel : public Layout {
            std::vector<Layout*> children;

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                // Handle different states, such as automatic sizing or automatic positioning
                Vector2f size = {
                    style.size.x.is_auto() ? 0 : style.size.x.get(ctx.bounds.width),
                    style.size.y.is_auto() ? 0 : style.size.y.get(ctx.bounds.height)
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

                ctx.bounds.x = position.x + marginsCalc.x;
                ctx.bounds.y = position.y + marginsCalc.y;
                ctx.bounds.width = style.size.x.get(ctx.bounds.width) - (marginsCalc.x - marginsCalc.z) + (paddingCalc.x + paddingCalc.z);
                ctx.bounds.height = style.size.y.get(ctx.bounds.height) - (marginsCalc.y - marginsCalc.w) + (paddingCalc.y + paddingCalc.w);

                Command cmd;
                cmd.type = Command::SPRITE;
                cmd.x = ctx.bounds.x;
                cmd.y = ctx.bounds.y;
                cmd.sprite.width = ctx.bounds.width;
                cmd.sprite.height = ctx.bounds.height;
                cmd.sprite.texture = nullptr;
                cmd.sprite.color = style.backgroundColor;
                commands.push_back(cmd);
                uint thisCmdIndex = commands.size() - 1;

                // Calculate children render commands
                size_t startCount = commands.size();

                for(Layout* ptr : children){
                    // Obtain render commands for children
                    ptr->render(ctx, commands);
                }

                size_t count = commands.size() - startCount;

                // With the count of objects which are the children commands, move the child to be relative to this layout object
                for(size_t i = 0; i < count; i++){
                    Command& cmd = commands[i + startCount];
                    cmd.x += ctx.bounds.x + paddingCalc.x;
                    cmd.y += ctx.bounds.y + paddingCalc.y;
                }

                // With the children now positioned, calculate the size for auto-sizing
                for(size_t i = 0; i < count; i++){
                    Command& cmd = commands[i + startCount];
                    
                    ctx.bounds.width += (cmd.x - ctx.bounds.x - paddingCalc.x);
                    ctx.bounds.height += (cmd.y - ctx.bounds.y - paddingCalc.y);

                    if(cmd.type == Command::SPRITE){
                        ctx.bounds.width += cmd.sprite.width;
                        ctx.bounds.height += cmd.sprite.height;
                    }
                }

                // Update auto sizing if enabled to fit contents of children
                Command& thisCmd = commands[thisCmdIndex];
                if(style.size.x.is_auto()){
                    thisCmd.x = ctx.bounds.x;
                    thisCmd.sprite.width = ctx.bounds.width;
                }

                if(style.size.y.is_auto()){
                    thisCmd.y = ctx.bounds.y;
                    thisCmd.sprite.height = ctx.bounds.height;
                }
            };
        };

        struct Label : public Layout {
            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                
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