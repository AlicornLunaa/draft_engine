#include "draft/interface/interface.hpp"
#include "draft/core/application.hpp"
#include "glm/common.hpp"
#include <vector>

namespace Draft::UI {
    struct DomTree {
        const Layout* layout = nullptr;
        std::vector<DomTree> leafs;
        Metrics metrics{};
    };

    // Constructors
    Interface::Interface(Scene* scene) : scene(scene), masterCtx(batch, textBatch, {}) {}

    // Functions
    void Interface::begin(){
        Vector2f winSize = scene->get_app()->window.get_size();
        masterCtx.bounds = {0, 0, winSize.x, winSize.y};

        camera = {
            camera.get_position(),
            camera.get_forward(),
            0, winSize.x,
            winSize.y, 0
        };

        batch.set_proj_matrix(camera.get_combined());
        batch.begin();
    }

    void build_tree(const Layout& layout, DomTree* node){
        if(!node) return;

        node->layout = &layout;
        node->metrics = {
            {
                .outer = {
                    0_pixels,
                    0_pixels,
                    true,
                    true
                },
                .inner = {
                    0_pixels,
                    0_pixels,
                    true,
                    true
                }
            },
            {
                .outer = {
                    0.f,
                    0.f,
                    0.f,
                    0.f
                },
                .inner = {
                    0.f,
                    0.f,
                    0.f,
                    0.f
                }
            }
        };

        for(const Layout* child : layout.get_children()){
            DomTree leaf;
            build_tree(*child, &leaf);
            node->leafs.push_back(leaf);
        }
    }
    void Interface::concrete_phase(DomTree* parent, DomTree* node, const Vector2f& winSize){
        if(!node) return;
        const GlobalStyle& style = node->layout->style;

        // Size holding the regular areas
        auto& outer = node->metrics.fluid.outer;
        outer.x += style.position.x;
        outer.y += style.position.y;
        outer.width = style.size.width + (style.padding.left + style.padding.right);
        outer.height = style.size.height + (style.padding.top + style.padding.bottom);

        auto& inner = node->metrics.fluid.inner;
        inner.x = outer.x + style.margins.left;
        inner.y = outer.y + style.margins.top;
        inner.width = style.size.width - (style.margins.left + style.margins.right) + (style.padding.left + style.padding.right);
        inner.height = style.size.height - (style.margins.top + style.margins.bottom) + (style.padding.top + style.padding.bottom);

        // If automatic sizing, read children
        if(style.size.width.is_auto()){
            float rightEdge = 0.f;

            for(DomTree& leaf : node->leafs){
                rightEdge = Math::max(rightEdge, leaf.layout->style.position.x.get(0.f) + leaf.layout->get_preferred_size(masterCtx).x.get(0.f));
            }

            outer.width += rightEdge;
            inner.width += rightEdge;
        }

        if(style.size.height.is_auto()){
            float bottomEdge = 0.f;

            for(DomTree& leaf : node->leafs){
                bottomEdge = Math::max(bottomEdge, leaf.layout->style.position.y.get(0.f) + leaf.layout->get_preferred_size(masterCtx).y.get(0.f));
            }

            outer.height += bottomEdge;
            inner.height += bottomEdge;
        }

        // Convert all to pixel units
        Vector4f parentPadding = {0, 0, 0, 0};
        Vector2f pos = {0, 0};
        Vector2f size = winSize;

        if(parent){
            parentPadding = parent->metrics.constraint.padding;
            pos.x = parent->metrics.constraint.inner.x;
            pos.y = parent->metrics.constraint.inner.y;
            size.x = parent->metrics.constraint.inner.width;
            size.y = parent->metrics.constraint.inner.height;
        }

        Vector4f marginsCalc = {
            style.margins.left.get(size.x),
            style.margins.top.get(size.x),
            style.margins.right.get(size.x),
            style.margins.bottom.get(size.x)
        };

        Vector4f paddingCalc = {
            style.padding.left.get(size.x),
            style.padding.top.get(size.x),
            style.padding.right.get(size.x),
            style.padding.bottom.get(size.x)
        };

        node->metrics.constraint.padding = paddingCalc;

        node->metrics.constraint.outer.x = node->metrics.fluid.outer.x.get(size.x);
        node->metrics.constraint.outer.y = node->metrics.fluid.outer.y.get(size.y);
        node->metrics.constraint.outer.width = node->metrics.fluid.outer.width.get(size.x);
        node->metrics.constraint.outer.height = node->metrics.fluid.outer.height.get(size.y);

        node->metrics.constraint.outer.width = Math::clamp(node->metrics.constraint.outer.width, style.size.minWidth.get(size.x), std::max(style.size.maxWidth.get(size.x) - parentPadding.x - parentPadding.w, 0.f));
        node->metrics.constraint.outer.height = Math::clamp(node->metrics.constraint.outer.height, style.size.minHeight.get(size.y), std::max(style.size.maxHeight.get(size.y) - parentPadding.y - parentPadding.z, 0.f));
        node->metrics.constraint.outer.x = Math::clamp(node->metrics.constraint.outer.x, style.size.minWidth.get(size.x) + pos.x + parentPadding.x, style.size.maxWidth.get(size.x) + pos.x - (node->metrics.constraint.outer.width + parentPadding.z));
        node->metrics.constraint.outer.y = Math::clamp(node->metrics.constraint.outer.y, style.size.minHeight.get(size.y) + pos.y + parentPadding.y, style.size.maxHeight.get(size.y) + pos.y - (node->metrics.constraint.outer.height + parentPadding.w));

        node->metrics.constraint.inner.x = node->metrics.constraint.outer.x + marginsCalc.x;
        node->metrics.constraint.inner.y = node->metrics.constraint.outer.y + marginsCalc.y;
        node->metrics.constraint.inner.width = node->metrics.constraint.outer.width - (marginsCalc.x + marginsCalc.z);
        node->metrics.constraint.inner.height = node->metrics.constraint.outer.height - (marginsCalc.y + marginsCalc.w);

        node->metrics.constraint.inner.width = Math::clamp(node->metrics.constraint.inner.width, style.size.minWidth.get(size.x), std::max(style.size.maxWidth.get(size.x), 0.f));
        node->metrics.constraint.inner.height = Math::clamp(node->metrics.constraint.inner.height, style.size.minHeight.get(size.y), std::max(style.size.maxHeight.get(size.y), 0.f));

        // Size children and move them relative to this
        for(DomTree& leaf : node->leafs){
            leaf.metrics.fluid.inner.x += node->metrics.constraint.inner.x + style.padding.left.get(size.x);
            leaf.metrics.fluid.inner.y += node->metrics.constraint.inner.y + style.padding.top.get(size.x);
            leaf.metrics.fluid.outer.x += node->metrics.constraint.inner.x + style.padding.left.get(size.x);
            leaf.metrics.fluid.outer.y += node->metrics.constraint.inner.y + style.padding.top.get(size.x);

            concrete_phase(node, &leaf, winSize);
        }
    }
    void render_phase(const Context& masterCtx, std::vector<Command>& commands, DomTree* node){
        if(!node) return;

        Command cmd;
        cmd.type = Command::SHAPE;
        cmd.color = Color::LIME;
        cmd.shape.pos = {node->metrics.constraint.outer.x, node->metrics.constraint.outer.y};
        cmd.shape.size = {node->metrics.constraint.outer.width, node->metrics.constraint.outer.height};
        commands.push_back(cmd);

        cmd.color = Color::CYAN;
        cmd.shape.pos = {node->metrics.constraint.inner.x, node->metrics.constraint.inner.y};
        cmd.shape.size = {node->metrics.constraint.inner.width, node->metrics.constraint.inner.height};
        commands.push_back(cmd);

        Context ctx = masterCtx;
        ctx.bounds = node->metrics.constraint.inner;
        node->layout->render(ctx, commands);

        for(DomTree& leaf : node->leafs){
            render_phase(masterCtx, commands, &leaf);
        }
    }
    void Interface::draw(const Layout& layout){
        // Tree construction
        DomTree tree;
        build_tree(layout, &tree);

        // Concrete phase
        concrete_phase(nullptr, &tree, scene->get_app()->window.get_size());

        // Render phase
        std::vector<Command> commands;
        render_phase(masterCtx, commands, &tree);

        // Draws this layout and all its children
        for(uint i = 0; i < commands.size(); i++){
            Command& cmd = commands[i];

            switch(cmd.type){
                case Command::SPRITE:
                    if(&batch.get_shader() != defaultShader.get_ptr()){
                        batch.set_shader(defaultShader);
                    }

                    batch.draw({
                        cmd.sprite.texture, {},
                        {cmd.x, cmd.y}, 0.f,
                        {cmd.sprite.width, cmd.sprite.height},
                        {0.f, 0.f}, 0.f,
                        cmd.color
                    });
                    break;

                case Command::TEXT:
                    textBatch.draw_text(batch, {
                        std::string(cmd.text.str),
                        cmd.text.font,
                        cmd.text.fontSize,
                        {cmd.x, cmd.y},
                        {},
                        cmd.color
                    });
                    break;

                case Command::SHAPE:
                    batch.end();
                    shapes.set_proj_matrix(camera.get_combined());
                    shapes.begin();
                    shapes.set_color(cmd.color);
                    shapes.draw_rect(cmd.shape.pos, cmd.shape.size, 0.f);
                    shapes.end();
                    batch.begin();
                    break;

                case Command::START_SCISSOR:
                    scissor.box.x = cmd.x;
                    scissor.box.y = cmd.y;
                    scissor.box.width = cmd.scissor.width;
                    scissor.box.height = cmd.scissor.height;
                    scissor.begin();
                    break;

                case Command::END_SCISSOR:
                    scissor.end();
                    break;
            }
        }
    }

    void Interface::end(){
        batch.end();
    }
};