#include <format>

#include "SFML/Window/Event.hpp"
#include "draft/core/application.hpp"
#include "draft/core/scene.hpp"
#include "draft/core/entity.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    Scene::Scene(Application* app) : app(app){
        this->uiCamera = sf::View(sf::FloatRect(0, 0, app->window.getSize().x, app->window.getSize().y));
    }

    entt::registry& Scene::getRegistry(){
        return registry;
    }

    Entity Scene::createEntity(){
        return { this, registry.create() };
    }

    void Scene::handleEvent(sf::Event event){
        switch(event.type){
            case sf::Event::MouseButtonPressed:
                Logger::println(Level::INFO, "Scene", std::format("Mouse clicked at x: {:1}, y: {:1}", event.mouseButton.x, event.mouseButton.y));
                break;

            default:
                break;
        }
    }

    void Scene::update(sf::Time deltaTime){
        // TODO: Implementation
    }

    void Scene::render(sf::Time deltaTime){
        app->console.draw();
        app->window.setView(uiCamera);
        ImGui::SFML::Render(app->window);
    }
}