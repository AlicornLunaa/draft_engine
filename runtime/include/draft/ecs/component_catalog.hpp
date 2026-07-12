#pragma once

#include "draft/ecs/entity.hpp"
#include "draft/util/reflectable.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    /**
     * @brief Type-erased base of ComponentTypeCatalogEntry<T>, letting ComponentCatalog hold a
     * heterogeneous collection of registered component types behind one type_index/name-keyed
     * lookup. Not for compile-time determination, just for loading at runtime.
     */
    struct ComponentTypeInterface {
        virtual ~ComponentTypeInterface() = default;

        virtual const std::string& name() const = 0;
        virtual std::type_index type() const = 0;

        /**
         * @brief Adds a default-constructed instance of this component to @p entity, replacing
         * any existing one.
         */
        virtual void add_default(Entity entity) const = 0;

        /**
         * @brief Removes this component type from @p entity, if present.
         */
        virtual void remove(Entity entity) const = 0;

        /**
         * @brief True if @p entity currently has this component type.
         */
        virtual bool has(Entity entity) const = 0;

        /**
         * @brief Copies this component from @p src to @p dst, replacing any existing instance on
         * @p dst. No-op if @p src doesn't have this component.
         */
        virtual void clone(Entity src, Entity dst) const = 0;
    };

    /**
     * @brief ComponentTypeInterface for a specific T, constructed only by
     * ComponentCatalog::register_component<T>() (see there for T's requirements).
     */
    template<typename T>
    class ComponentTypeCatalogEntry : public ComponentTypeInterface {
    public:
        explicit ComponentTypeCatalogEntry(std::string name) : m_name(std::move(name)) {}

        const std::string& name() const override { return m_name; }
        std::type_index type() const override { return std::type_index(typeid(T)); }

        void add_default(Entity entity) const override { entity.add_component<T>(); }
        void remove(Entity entity) const override { entity.remove_component<T>(); }
        bool has(Entity entity) const override { return entity.has_component<T>(); }

        void clone(Entity src, Entity dst) const override {
            if(T* source = src.try_get_component<T>())
                dst.add_component<T>(*source);
        }

    private:
        std::string m_name;
    };

    /**
     * @brief Process-wide catalog of known component types. Not a container of live
     * components (see Registry/Entity for that). Types are registered explicitly, by name, via
     * register_component<T>(); scene loading, the inspector, and the prefab system then look
     * entries up generically by name (from a file) or by type_index (from a live component).
     */
    class ComponentCatalog {
    public:
        ComponentCatalog() = default;
        ComponentCatalog(const ComponentCatalog&) = delete;
        ComponentCatalog& operator=(const ComponentCatalog&) = delete;

        /**
         * @brief Registers T under its reflected name (T::reflect_name()), replacing any
         * previously-registered T (keeping its original position in all()) but leaving every
         * other registered type untouched.
         * @throws std::logic_error if T's name is already registered to a *different* type.
         */
        template<typename T>
        void register_component(){
            static_assert(Reflectable<T>, "ComponentCatalog::register_component<T>(): T must be Reflectable (see DRAFT_REFLECTABLE)");
            static_assert(std::is_default_constructible_v<T>, "ComponentCatalog::register_component<T>(): T must be default-constructible");
            static_assert(std::is_copy_constructible_v<T>, "ComponentCatalog::register_component<T>(): T must be copy-constructible");

            auto type = std::type_index(typeid(T));
            std::string name(T::reflect_name());

            auto nameIter = m_byName.find(name);
            if(nameIter != m_byName.end() && nameIter->second->type() != type)
                throw std::logic_error("ComponentCatalog::register_component(): name '" + name + "' is already registered to a different component type");

            auto entry = std::make_unique<ComponentTypeCatalogEntry<T>>(name);
            ComponentTypeInterface* ptr = entry.get();

            auto typeIt = m_byType.find(type);
            if(typeIt == m_byType.end()){
                m_order.push_back(ptr);
            }else{
                m_byName.erase(typeIt->second->name());
                std::replace(m_order.begin(), m_order.end(), static_cast<ComponentTypeInterface*>(typeIt->second.get()), ptr);
            }

            m_byName[ptr->name()] = ptr;
            m_byType[type] = std::move(entry);
        }

        /**
         * @brief Gets the catalog entry for T, or nullptr if T isn't registered.
         */
        template<typename T>
        ComponentTypeInterface* by_type() const {
            return by_type(std::type_index(typeid(T)));
        }

        /**
         * @brief Gets the catalog entry for @p type, or nullptr if nothing is registered under it.
         */
        ComponentTypeInterface* by_type(std::type_index type) const {
            auto it = m_byType.find(type);
            return it == m_byType.end() ? nullptr : it->second.get();
        }

        /**
         * @brief Gets the catalog entry registered under @p name, or nullptr if none is.
         */
        ComponentTypeInterface* by_name(std::string_view name) const {
            auto it = m_byName.find(std::string(name));
            return it == m_byName.end() ? nullptr : it->second;
        }

        /**
         * @brief Every registered entry, in registration order (stable across replacement, see
         * register_component()). Meant for listing "known" component types, e.g. an editor's Add
         * Component menu.
         */
        const std::vector<ComponentTypeInterface*>& all() const { return m_order; }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<ComponentTypeInterface>> m_byType;
        std::unordered_map<std::string, ComponentTypeInterface*> m_byName;
        std::vector<ComponentTypeInterface*> m_order;
    };
}
