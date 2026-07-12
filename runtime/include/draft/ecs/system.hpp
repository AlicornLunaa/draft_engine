#pragma once

#include "draft/input/event.hpp"
#include "draft/rendering/render_layer.hpp"
#include "draft/util/time.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    /**
     * @brief Base of anything registered into a SystemRegistry. A per-tick behavior attached
     * to a Scene (physics, rendering, gameplay logic, ...).
     *
     * This isn't for ECS-internal bookkeeping that reacts to component construction/destruction
     * rather than running per-tick (see RelationshipSystem, which Scene owns directly instead).
     */
    class AbstractSystem {
    public:
        virtual ~AbstractSystem() = default;

        /**
         * @brief Advances this system by a fixed-size @p dt. Called zero or more times per
         * frame (accumulator-style) by SystemRegistry::update_all() / Scene::update(), for
         * anything that needs a stable, deterministic step.
         *
         * Default no-op: a system only interested in per-frame work (see render()) doesn't
         * need to override this.
         */
        virtual void update(Time dt) {}

        /**
         * @brief Advances this system by the actual, variable frame @p dt, for whichever
         * @p layer this call is for. Called by SystemRegistry::render_all(dt, layer) /
         * Scene::render(dt, layer), once per frame per layer this system participates in (see
         * get_render_layers()), for anything that should run every frame regardless of how many
         * (or how few) fixed update() steps just ran.
         *
         * RenderLayer::Default (the default of get_render_layers()) is driven directly by
         * Application::frame(), before Renderer::render_frame() runs at all, and always runs
         * even with no Renderer set.
         *
         * Default no-op: a system only interested in fixed-step work doesn't need to override
         * this.
         */
        virtual void render(Time dt, RenderLayer layer) {}

        /**
         * @brief Which RenderLayer(s) this system's render(dt, layer) should be called for.
         * Defaults to RenderLayer::Default, matching the pre-RenderLayer behavior of render()
         * being called exactly once per frame regardless of rendering.
         */
        virtual RenderLayer get_render_layers() const { return RenderLayer::Default; }

        /**
         * @brief Called once when this system's Scene becomes the active one (see
         * SystemRegistry::attach_all() / Scene::attach()), e.g. starting ambient music.
         *
         * Default no-op.
         */
        virtual void on_attach() {}

        /**
         * @brief Called once when this system's Scene stops being the active one (see
         * SystemRegistry::detach_all() / Scene::detach()), e.g. stopping ambient music.
         *
         * Default no-op.
         */
        virtual void on_detach() {}

        /**
         * @brief Handles an input/window @p event. Called by SystemRegistry::dispatch_event() /
         * Scene::dispatch_event(), in registration order, stopping at the first system that
         * returns true, e.g. an ImGui-integration system claiming an event ImGui itself wants
         * (WantCaptureMouse/WantCaptureKeyboard) so nothing registered after it sees that input.
         *
         * @return True if this system consumed the event (stop propagating it further).
         * Default no-op, returns false.
         */
        virtual bool on_event(const Event& event) { return false; }
    };

    /**
     * @brief Owns a Scene's systems, keyed by type.
     *
     * Construction arguments are forwarded to T's constructor a system that needs its owning Scene
     * (or anything else) takes it as a constructor parameter, e.g. `systems.add<PhysicsSystem>(scene)`.
     */
    class SystemRegistry {
    public:
        SystemRegistry() = default;
        SystemRegistry(const SystemRegistry&) = delete;
        SystemRegistry& operator=(const SystemRegistry&) = delete;

        /**
         * @brief Constructs and registers a T, replacing any previously-registered T.
         * @return A reference to the newly-constructed system.
         */
        template<typename T, typename... Args>
        T& add(Args&&... args){
            static_assert(std::is_base_of_v<AbstractSystem, T>, "SystemRegistry::add<T>(): T must derive from AbstractSystem");

            auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *ptr;

            auto type = std::type_index(typeid(T));
            if(!m_systems.contains(type))
                m_order.push_back(type);

            m_systems[type] = std::move(ptr);
            return ref;
        }

        /**
         * @brief Gets the registered T.
         * @throws std::logic_error if no T is registered.
         */
        template<typename T>
        T& get(){
            auto it = m_systems.find(std::type_index(typeid(T)));
            if(it == m_systems.end())
                throw std::logic_error("SystemRegistry::get(): no system of the requested type is registered");

            return static_cast<T&>(*it->second);
        }

        /**
         * @brief Gets the registered T, or nullptr if none is registered.
         */
        template<typename T>
        T* try_get(){
            auto it = m_systems.find(std::type_index(typeid(T)));
            return it == m_systems.end() ? nullptr : static_cast<T*>(it->second.get());
        }

        template<typename T>
        bool has() const {
            return m_systems.contains(std::type_index(typeid(T)));
        }

        /**
         * @brief Removes the registered T.
         * @return False if no T was registered.
         */
        template<typename T>
        bool remove(){
            auto type = std::type_index(typeid(T));
            auto it = m_systems.find(type);
            if(it == m_systems.end())
                return false;

            m_systems.erase(it);
            std::erase(m_order, type);
            return true;
        }

        /**
         * @brief Calls update(dt) on every registered system, in the order each was first
         * added. Meant to be driven at a fixed timestep, possibly several times (or not at all)
         * per frame.
         */
        void update_all(Time dt);

        /**
         * @brief Calls render(dt, layer) on every registered system whose get_render_layers()
         * includes @p layer, in the order each was first added. Meant to be driven once per
         * frame per layer (see AbstractSystem::render()'s doc comment for when each layer runs).
         */
        void render_all(Time dt, RenderLayer layer);

        /**
         * @brief Calls on_attach() on every registered system, in registration order. Meant to
         * be driven once when this registry's Scene becomes the active one.
         */
        void attach_all();

        /**
         * @brief Calls on_detach() on every registered system, in registration order. Meant to
         * be driven once when this registry's Scene stops being the active one.
         */
        void detach_all();

        /**
         * @brief Calls on_event(event) on every registered system, in registration order,
         * stopping at (and returning true from) the first one that returns true. See
         * Scene::dispatch_event().
         * @return True if some system consumed the event.
         */
        bool dispatch_event(const Event& event);

    private:
        std::unordered_map<std::type_index, std::unique_ptr<AbstractSystem>> m_systems;
        std::vector<std::type_index> m_order;
    };
}
