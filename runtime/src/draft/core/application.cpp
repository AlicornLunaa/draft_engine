#include "draft/core/application.hpp"
#include "draft/input/action.hpp"

#include <algorithm>

namespace Draft {
    // Constructors
    Application::Application(const std::string& title, unsigned int width, unsigned int height) : window(width, height, title), keyboard(window), mouse(window) {
        window.frameSizeCallback = [this](unsigned int w, unsigned int h){ framebuffer_resized(w, h); };
        window.focusCallback = [this](bool focused){ window_focus(focused); };
        window.closeCallback = [this](){ window_closed(); };
        keyboard.keyCallback = [this](int key, int action, int modifier){ key_callback(key, action, modifier); };
        keyboard.textCallback = [this](unsigned int codepoint){ text_callback(codepoint); };
        mouse.mouseButtonCallback = [this](int button, int action, int modifier){ mouse_button_callback(button, action, modifier); };
        mouse.mousePosCallback = [this](const Vector2d& position){ mouse_position_callback(position); };
        mouse.mouseScrollCallback = [this](const Vector2d& delta){ mouse_scroll_callback(delta); };
        mouse.mouseEnterCallback = [this](){ mouse_enter_callback(); };
        mouse.mouseLeaveCallback = [this](){ mouse_leave_callback(); };

        p_renderer = std::make_unique<DefaultRenderer>(window.get_size());
    }

    // Private functions
    bool Application::dispatch(const Event& event){
        if(eventCallback && eventCallback(event))
            return true;

        if(p_activeScene)
            return p_activeScene->dispatch_event(event);

        return false;
    }

    void Application::framebuffer_resized(unsigned int width, unsigned int height){
        p_pendingResize = true;
    }

    void Application::window_focus(bool focused){
        Event event;
        event.type = focused ? Event::GainedFocus : Event::LostFocus;
        dispatch(event);
    }

    void Application::window_closed(){
        Event event;
        event.type = Event::Closed;
        dispatch(event);
    }

    void Application::key_callback(int key, int action, int modifier){
        Event event;
        event.type = (action == Action::PRESS) ? Event::KeyPressed : (action == Action::RELEASE) ? Event::KeyReleased : Event::KeyHold;
        event.key.code = key;
        event.key.alt = modifier & static_cast<int>(Keyboard::Modifier::ALT);
        event.key.control = modifier & static_cast<int>(Keyboard::Modifier::CTRL);
        event.key.shift = modifier & static_cast<int>(Keyboard::Modifier::SHIFT);
        event.key.system = modifier & static_cast<int>(Keyboard::Modifier::SUPER);
        event.key.mods = modifier;
        dispatch(event);
    }

    void Application::text_callback(unsigned int codepoint){
        Event event;
        event.type = Event::TextEntered;
        event.text.unicode = codepoint;
        dispatch(event);
    }

    void Application::mouse_button_callback(int button, int action, int modifier){
        Event event;
        event.type = (action == Action::PRESS) ? Event::MouseButtonPressed : Event::MouseButtonReleased;
        const Vector2d& position = mouse.get_position();
        event.mouseButton.button = button;
        event.mouseButton.x = (int)position.x;
        event.mouseButton.y = (int)position.y;
        event.mouseButton.mods = modifier;
        dispatch(event);
    }

    void Application::mouse_position_callback(const Vector2d& position){
        Event event;
        event.type = Event::MouseMoved;
        event.mouseMove.x = (int)position.x;
        event.mouseMove.y = (int)position.y;
        dispatch(event);
    }

    void Application::mouse_scroll_callback(const Vector2d& delta){
        Event event;
        event.type = Event::MouseWheelScrolled;
        event.mouseWheelScroll.x = delta.x;
        event.mouseWheelScroll.y = delta.y;
        dispatch(event);
    }

    void Application::mouse_enter_callback(){
        Event event;
        event.type = Event::MouseEntered;
        dispatch(event);
    }

    void Application::mouse_leave_callback(){
        Event event;
        event.type = Event::MouseLeft;
        dispatch(event);
    }

    void Application::trigger_resize(unsigned int width, unsigned int height){
        if(p_renderer)
            p_renderer->resize({width, height});

        Event event;
        event.type = Event::Resized;
        event.size.width = width;
        event.size.height = height;
        dispatch(event);

        p_pendingResize = false;
    }

    void Application::scene_change(){
        if(p_activeScene)
            p_activeScene->detach();

        reset_timers();

        p_activeScene = p_newScene;
        p_newScene = nullptr;

        if(p_activeScene)
            p_activeScene->attach();
    }

    void Application::reset_timers(){
        p_accumulator = 0.0;
        p_deltaClock.restart();
        deltaTime = Time();
    }

    void Application::tick(){
        if(simulationPaused){
            p_accumulator = 0.0;
            return;
        }

        p_accumulator += deltaTime.as_seconds();
        p_accumulator = std::min(p_accumulator, (double)maxAccumulator.as_seconds());

        while(p_accumulator >= timeStep.as_seconds()){
            if(p_activeScene)
                p_activeScene->update(timeStep);

            p_accumulator -= timeStep.as_seconds();
        }
    }

    void Application::frame(){
        window.clear();

        if(p_activeScene){
            // Ordinary per-frame work always runs, even with no renderer set
            p_activeScene->render(deltaTime, RenderLayer::Default);

            if(p_renderer)
                p_renderer->render_frame(deltaTime, p_activeScene->get_systems());
        }

        window.display();
    }

    // Functions
    void Application::run(){
        while(step());
    }

    bool Application::step(){
        deltaTime = p_deltaClock.restart();

        if(p_pendingResize){
            Vector2u size = window.get_frame_size();
            trigger_resize(size.x, size.y);
        }

        if(p_newRenderer){
            p_renderer = std::move(p_newRenderer);
            p_newRenderer = nullptr;
        }

        if(p_newScene)
            scene_change();

        window.poll_events();

        tick();
        frame();

        return window.is_open();
    }

    void Application::set_scene(Scene* scene){
        p_newScene = scene;
    }

    void Application::set_renderer(std::unique_ptr<Renderer> renderer){
        p_newRenderer = std::move(renderer);
    }
}
