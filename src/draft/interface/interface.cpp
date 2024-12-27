#include "draft/interface/interface.hpp"
#include "draft/core/application.hpp"

namespace Draft::UI {
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

    void Interface::draw(const Layout& layout){
        // Draws this layout and all its children
        std::vector<Command> commands;
        layout.render(masterCtx, commands);

        // Render all commands given
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
            }
        }
    }

    void Interface::end(){
        batch.end();
    }
};