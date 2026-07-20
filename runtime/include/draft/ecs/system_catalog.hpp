#pragma once

#include "draft/ecs/scene.hpp"
#include "draft/ecs/system.hpp"
#include "draft/util/reflectable.hpp"
#include "draft/util/serialization/serializer.hpp"

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
    /** @brief Used to construct a system using closures. Returns the system for the given scene. */
    typedef std::function<std::unique_ptr<AbstractSystem>(Scene&)> SystemFactory;

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
         * @brief Constructs this system via its factory (see SystemFactory) for @p scene and
         * adds it to scene.get_systems().
         */
        virtual void add(Scene& scene) const = 0;

        /**
         * @brief Removes this system type from @p registry.
         */
        virtual void remove(SystemRegistry& registry) const = 0;

        /**
         * @brief True if @p entity currently has this component type.
         */
        virtual bool has(const SystemRegistry& registry) const = 0;

        /**
         * @brief Serializes this system's own reflected fields off the instance already attached
         * to @p registry, which must already have(registry). A system's construction dependencies
         * (World&, Renderer&, ...) live in its factory closure instead, see add(), and are never part of this.
         */
        virtual void serialize(const SystemRegistry& registry, JSON& json) const = 0;
        virtual void serialize(const SystemRegistry& registry, Binary::ByteArray& out) const = 0;

        /**
         * @brief Deserializes onto the instance already attached to @p registry (see add(),
         * which must be called first, unlike a component, a system can't be default-constructed
         * on demand here, its real dependencies only exist in the factory closure).
         * @throws std::logic_error if @p registry doesn't already have this system.
         */
        virtual void deserialize(SystemRegistry& registry, JSON& json) const = 0;
        virtual void deserialize(SystemRegistry& registry, Binary::ByteView data) const = 0;
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

        void add(Scene& scene) const override { scene.get_systems().emplace<T>(m_factory(scene)); }
        void remove(SystemRegistry& registry) const override { registry.remove<T>(); }
        bool has(const SystemRegistry& registry) const override { return registry.has<T>(); }

        void serialize(const SystemRegistry& registry, JSON& json) const override { Serializer::serialize(registry.get<T>(), json); }
        void serialize(const SystemRegistry& registry, Binary::ByteArray& out) const override { Serializer::serialize(registry.get<T>(), out); }

        void deserialize(SystemRegistry& registry, JSON& json) const override { Serializer::deserialize(registry.get<T>(), json); }
        void deserialize(SystemRegistry& registry, Binary::ByteView data) const override { Serializer::deserialize(registry.get<T>(), data); }

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

        /**
         * @brief Drops every registered entry. Needed before unloading a dynamically-loaded game
         * module (see GameModuleLoader): a SystemTypeCatalogEntry<T> registered from inside that
         * module has its vtable, and its factory closure's code, compiled into the module itself,
         * so once the module is unloaded, touching an entry left over from it is a dangling call
         * into unmapped memory. Must be called while that module is still loaded, before it's
         * unloaded.
         */
        void clear(){
            m_byType.clear();
            m_byName.clear();
            m_order.clear();
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<SystemTypeInterface>> m_byType;
        std::unordered_map<std::string, SystemTypeInterface*> m_byName;
        std::vector<SystemTypeInterface*> m_order;
    };
}
