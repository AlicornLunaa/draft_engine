#pragma once

#include "draft/core/scene.hpp"
#include "draft/interface/unit_value.hpp"
#include "draft/math/glm.hpp"
#include "draft/math/rect.hpp"
#include "draft/rendering/batching/shape_batch.hpp"
#include "draft/rendering/batching/sprite_batch.hpp"
#include "draft/rendering/batching/text_renderer.hpp"
#include "draft/rendering/camera.hpp"
#include "draft/rendering/shader.hpp"
#include "draft/util/asset_manager/asset_manager.hpp"
#include "draft/util/asset_manager/resource.hpp"
#include "draft/util/color.hpp"

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
            UnitValue maxWidth = 100_percent;

            UnitValue height = UnitValue::Auto;
            UnitValue minHeight = 0_pixels;
            UnitValue maxHeight = 100_percent;
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
        typedef Rect<UnitValue> UnitRect;

        struct Command {
            enum Type { SPRITE, TEXT, SHAPE } type;

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

                struct {
                    Vector2f pos;
                    Vector2f size;
                } shape;
            };
        };

        struct Context {
            SpriteBatch& batch;
            TextRenderer& textBatch;
            FloatRect bounds; // The inner container size
        };

        struct Metrics {
            struct {
                // Concrete phase consists of measuring in percentages and pixels.
                // Fluid phase resizes parents to fit concrete children if needed
                UnitRect outer = UnitRect(true, true, true, true);
                UnitRect inner = UnitRect(true, true, true, true);
            } fluid;

            struct {
                // Constraint phase resizes elements to fit strict min/max sizes
                // Render phase is the final output
                FloatRect outer{};
                FloatRect inner{};
                Vector4f padding{};
            } constraint;
        };

        // Widgets
        struct Layout {
            GlobalStyle style;

            virtual const std::vector<Layout*> get_children() const { return {}; }
            virtual const Vector2<UnitValue> get_preferred_size(const Context& ctx) const { return { style.size.width, style.size.height }; }
            virtual void render(Context ctx, std::vector<Command>& commands) const = 0;
        };

        struct Panel : public Layout {
            // Lays children out absolutely
            std::vector<Layout*> children;

            virtual const std::vector<Layout*> get_children() const override { return children; }

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                Command cmd;
                cmd.type = Command::SPRITE;
                cmd.x = ctx.bounds.x;
                cmd.y = ctx.bounds.y;
                cmd.color = style.backgroundColor;
                cmd.sprite.width = ctx.bounds.width;
                cmd.sprite.height = ctx.bounds.height;
                cmd.sprite.texture = nullptr;
                commands.push_back(cmd);
            };
        };

        struct Grid : public Layout {
            // Data structures
            struct Item {
                Layout* child = nullptr;
                uint columnSpan = 1;
            };

            // Lays children out in rows and columns, autosizing is not supported
            std::vector<Item> items;
            uint columns = 12;

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                // Get the size and position of the inner box
                // auto [content, outer] = get_content_box(ctx);

                // Command cmd;
                // cmd.type = Command::SPRITE;
                // cmd.x = content.x;
                // cmd.y = content.y;
                // cmd.color = style.backgroundColor;
                // cmd.sprite.width = content.width;
                // cmd.sprite.height = content.height;
                // cmd.sprite.texture = nullptr;
                // commands.push_back(cmd);

                // // Calculate children render commands
                // size_t startCount = commands.size();
                // float cellWidth = content.width / ((float)columns);
                // uint currentColumn = 0;

                // for(const Item& item : items){
                //     // Prevent rendering off the grid
                //     if(currentColumn > columns) break;

                //     // Calculate content size
                //     float width = cellWidth * item.columnSpan;
                //     currentColumn += item.columnSpan;

                //     // Create context for this element
                //     Context newCtx(ctx.batch, ctx.textBatch, {0, 0, content.width, content.height});

                //     // Obtain render commands for children
                //     item.child->render(newCtx, commands);
                // }

                // size_t count = commands.size() - startCount;

                // // With the count of objects which are the children commands, move the child to be relative to this layout object
                // Vector2f topLeftPadding(style.padding.left.get(ctx.bounds.width), style.padding.top.get(ctx.bounds.width));

                // for(size_t i = 0; i < count; i++){
                //     Command& cmd = commands[i + startCount];
                //     cmd.x += content.x + topLeftPadding.x; // Padding is needed here to push it off the side
                //     cmd.y += content.y + topLeftPadding.y;
                // }
            };
        };

        struct Label : public Layout {
            std::string str = "Hello World!";
            Font* font = nullptr;
            uint fontSize = 22;

            virtual const Vector2<UnitValue> get_preferred_size(const Context& ctx) const override {
                Vector2f size = ctx.textBatch.get_text_bounds({ str, font, fontSize });
                return { size.x, size.y };
            }

            virtual void render(Context ctx, std::vector<Command>& commands) const override {
                // Render text in the center if the given box
                Command cmd;
                cmd.type = Command::TEXT;
                cmd.x = ctx.bounds.x;
                cmd.y = ctx.bounds.y;
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
                Command cmd;
                cmd.type = Command::SPRITE;
                cmd.x = ctx.bounds.x;
                cmd.y = ctx.bounds.y;
                cmd.color = style.backgroundColor;
                cmd.sprite.width = ctx.bounds.width;
                cmd.sprite.height = ctx.bounds.height;
                cmd.sprite.texture = nullptr;
                commands.push_back(cmd);

                btnLabel.render(ctx, commands);
            };
        };

        // Main control panel for rendering
        struct DomTree;

        class Interface {
        private:
            // Variables
            Scene* scene = nullptr;
            SpriteBatch batch;
            ShapeBatch shapes;
            TextRenderer textBatch;
            Resource<Shader> defaultShader = Assets::manager.get<Shader>("assets/shaders/default");

            OrthoCamera camera = {{0, 0, 10}, {0, 0, -1}, 0, 1280, 720, 0};
            Context masterCtx;

            void concrete_phase(DomTree* parent, DomTree* node, const Vector2f& winSize);

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