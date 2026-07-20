#pragma once

#include "draft/core/command_catalog.hpp"
#include "draft/ecs/component_catalog.hpp"
#include "draft/ecs/system_catalog.hpp"

namespace Draft {
    /**
     * @brief Owns the engine's process-wide catalog(s) of known game types. What a compiled
     * game explicitly registers during RegisterGame(Engine&), and what scene loading, the
     * inspector, and the prefab system later query by name or by type. Not the runtime loop
     * itself, see Application for that.
     *
     * Pre-registers a small set of engine-owned component types so every
     * project can rely on them existing without registering them itself.
     */
    class Engine {
    public:
        Engine();
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        /**
         * @brief Registers T under its reflected name. Shorthand for
         * components().register_component<T>().
         */
        template<typename T>
        void register_component(){
            m_components.register_component<T>();
        }

        ComponentCatalog& components() { return m_components; }
        const ComponentCatalog& components() const { return m_components; }

        /**
         * @brief Registers T under its reflected name. Shorthand for
         * systems().register_system<T>().
         */
        template<typename T>
        void register_system(){
            m_systems.register_system<T>();
        }

        SystemCatalog& systems() { return m_systems; }
        const SystemCatalog& systems() const { return m_systems; }

        /**
         * @brief Shorthand for commands().register_command().
         */
        void register_command(CommandInfo info){
            m_commands.register_command(std::move(info));
        }

        CommandCatalog& commands() { return m_commands; }
        const CommandCatalog& commands() const { return m_commands; }

    private:
        ComponentCatalog m_components;
        SystemCatalog m_systems;
        CommandCatalog m_commands;
    };
}
