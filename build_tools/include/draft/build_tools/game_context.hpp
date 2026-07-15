#pragma once

#include "draft/asset/asset_manager.hpp"
#include "draft/core/application.hpp"
#include "draft/core/engine.hpp"

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>

namespace Draft {
    namespace detail {
        struct PersistentSlotInterface {
            virtual ~PersistentSlotInterface() = default;
        };

        template<typename T>
        struct PersistentSlot : PersistentSlotInterface {
            template<typename... Args>
            explicit PersistentSlot(Args&&... args) : value(std::forward<Args>(args)...) {}

            T value;
        };
    }

    /**
     * @brief Holds objects that need to live for the whole run rather than just one Scene, like
     * a physics World that a PhysicsSystem factory closure keeps referencing across scene
     * reloads. GameContext owns one of these, so anything stored here survives a scene reload
     * since that only ever touches the Scene itself, never GameContext.
     */
    class PersistentStore {
    public:
        /**
         * @brief Constructs a T in place and stores it, replacing any previously stored T.
         */
        template<typename T, typename... Args>
        T& emplace(Args&&... args){
            auto slot = std::make_unique<detail::PersistentSlot<T>>(std::forward<Args>(args)...);
            T& ref = slot->value;
            m_slots[std::type_index(typeid(T))] = std::move(slot);
            return ref;
        }

        /**
         * @brief Gets the stored T.
         * @throws std::logic_error if no T is stored.
         */
        template<typename T>
        T& get(){
            auto it = m_slots.find(std::type_index(typeid(T)));
            if(it == m_slots.end())
                throw std::logic_error("PersistentStore::get(): no object of the requested type is stored");

            return static_cast<detail::PersistentSlot<T>&>(*it->second).value;
        }

        /**
         * @brief Gets the stored T, or nullptr if none is stored.
         */
        template<typename T>
        T* try_get(){
            auto it = m_slots.find(std::type_index(typeid(T)));
            if(it == m_slots.end())
                return nullptr;

            return &static_cast<detail::PersistentSlot<T>&>(*it->second).value;
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<detail::PersistentSlotInterface>> m_slots;
    };

    /**
     * @brief Everything a game module's registration entry point gets (see DRAFT_GAME_MODULE).
     * The launcher owns one of these for the whole process, so a module can freely reach back
     * into application/engine/assets, or stash something in persistent, without worrying about
     * any of it going away before the process does.
     */
    struct GameContext {
        Engine& engine;
        Application& application;
        AssetManager& assets;
        PersistentStore persistent;
    };
}
