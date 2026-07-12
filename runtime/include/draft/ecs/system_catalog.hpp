#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace Draft {
    /** @brief Used to construct a system using closures */
    typedef std::function<std::unique_ptr<AbstractSystem>()> SystemFactory;

    /**
     * @brief Type-erased base of SystemTypeCatalogEntry<T>, letting SystemCatalog hold a
     * heterogeneous collection of registered system types behind one type_index/name-keyed
     * lookup. Not for compile-time determination, just for loading at runtime.
     */
    struct SystemTypeInterface {
        virtual ~SystemTypeInterface() = default;

        virtual const std::string& name() const = 0;
        virtual std::type_index type() const = 0;

        /**
         * @brief Adds a system type to the @p registry.
         */
        virtual void add(SystemRegistry& registry) const = 0;

        /**
         * @brief Removes this system type from @p registry.
         */
        virtual void remove(SystemRegistry& registry) const = 0;

        /**
         * @brief True if @p entity currently has this component type.
         */
        virtual bool has(const SystemRegistry& registry) const = 0;
    };

    /**
     * @brief SystemTypeInterface for a specific T, constructed only by
     * SystemCatalog::register_system<T>() (see there for T's requirements).
     */
    template<typename T>
    class SystemTypeCatalogEntry : public SystemTypeInterface {
    public:
        explicit SystemTypeCatalogEntry(std::string name, SystemFactory factory) : m_name(std::move(name)), m_factory(std::move(factory)) {}

        const std::string& name() const override { return m_name; }
        std::type_index type() const override { return std::type_index(typeid(T)); }

        void add(SystemRegistry& registry) const override { registry.emplace<T>(std::move(m_factory())); }
        void remove(SystemRegistry& registry) const override { registry.remove<T>(); }
        bool has(const SystemRegistry& registry) const override { return registry.has<T>(); }

    private:
        std::string m_name;
        SystemFactory m_factory;
    };

    /**
     * @brief Process-wide catalog of known system types. Not a container of live
     * systems (see SystemRegistry for that). Types are registered explicitly, by name, via
     * register_system<T>(); scene loading, the inspector, and the prefab system then look
     * entries up generically by name (from a file) or by type_index (from a live system).
     */
    class SystemCatalog {
    public:
        SystemCatalog() = default;
        SystemCatalog(const SystemCatalog&) = delete;
        SystemCatalog& operator=(const SystemCatalog&) = delete;

        /**
         * @brief Registers T under its reflected name (T::reflect_name()), replacing any
         * previously-registered T (keeping its original position in all()) but leaving every
         * other registered type untouched.
         * @throws std::logic_error if T's name is already registered to a *different* type.
         */
        template<typename T>
        void register_system(SystemFactory factory){
            static_assert(Reflectable<T>, "SystemCatalog::register_system<T>(): T must be Reflectable (see DRAFT_REFLECTABLE)");

            auto type = std::type_index(typeid(T));
            std::string name(T::reflect_name());

            auto nameIter = m_byName.find(name);
            if(nameIter != m_byName.end() && nameIter->second->type() != type)
                throw std::logic_error("SystemCatalog::register_system(): name '" + name + "' is already registered to a different component type");

            auto entry = std::make_unique<SystemTypeCatalogEntry<T>>(name, factory);
            SystemTypeInterface* ptr = entry.get();

            auto typeIter = m_byType.find(type);
            if(typeIter == m_byType.end()){
                m_order.push_back(ptr);
            }else{
                m_byName.erase(typeIter->second->name());
                std::replace(m_order.begin(), m_order.end(), static_cast<SystemTypeInterface*>(typeIter->second.get()), ptr);
            }

            m_byName[ptr->name()] = ptr;
            m_byType[type] = std::move(entry);
        }

        /**
         * @brief Gets the catalog entry for T, or nullptr if T isn't registered.
         */
        template<typename T>
        SystemTypeInterface* by_type() const {
            return by_type(std::type_index(typeid(T)));
        }

        /**
         * @brief Gets the catalog entry for @p type, or nullptr if nothing is registered under it.
         */
        SystemTypeInterface* by_type(std::type_index type) const {
            auto it = m_byType.find(type);
            return it == m_byType.end() ? nullptr : it->second.get();
        }

        /**
         * @brief Gets the catalog entry registered under @p name, or nullptr if none is.
         */
        SystemTypeInterface* by_name(std::string_view name) const {
            auto it = m_byName.find(std::string(name));
            return it == m_byName.end() ? nullptr : it->second;
        }

        /**
         * @brief Every registered entry, in registration order (stable across replacement, see
         * register_system()). Meant for listing "known" system types, e.g. an editor's Add
         * System menu.
         */
        const std::vector<SystemTypeInterface*>& all() const { return m_order; }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<SystemTypeInterface>> m_byType;
        std::unordered_map<std::string, SystemTypeInterface*> m_byName;
        std::vector<SystemTypeInterface*> m_order;
    };
}
