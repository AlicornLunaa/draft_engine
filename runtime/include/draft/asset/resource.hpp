#pragma once

#include <atomic>
#include <memory>

namespace Draft {
    /**
     * @brief A shareable, swappable cell holding a `shared_ptr<T>`.
     *
     * This is the indirection that makes Resource<T> "pointer-ish". AssetManager keeps one
     * AssetSlot per loaded asset, and every Resource<T> handed out for that asset shares
     * ownership of the *slot*, not of a particular T instance. Reloading an asset calls set()
     * on the same slot, so every outstanding Resource<T> observes the new value on its next
     * get(). No consumer needs to re-request the resource from the manager after a hot-reload.
     */
    template<typename T>
    class AssetSlot {
    public:
        AssetSlot() = default;
        explicit AssetSlot(std::shared_ptr<T> value) : m_value(std::move(value)) {}

        AssetSlot(const AssetSlot&) = delete;
        AssetSlot& operator=(const AssetSlot&) = delete;

        /**
         * @brief Gets the slot's current value (may be null, e.g. after unload()).
         */
        std::shared_ptr<T> get() const { return m_value.load(); }

        /**
         * @brief Replaces the slot's value; every Resource<T> sharing this slot observes the
         * change on its next get()/operator->().
         */
        void set(std::shared_ptr<T> value) { m_value.store(std::move(value)); }

    private:
        std::atomic<std::shared_ptr<T>> m_value;
    };

    /**
     * @brief A lightweight, copyable handle to an asset managed by AssetManager.
     *
     * Resource<T> holds a `shared_ptr<AssetSlot<T>>`, not a `shared_ptr<T>` directly. Copying
     * a Resource is cheap, and every copy keeps seeing the same underlying asset even across
     * AssetManager reloading it in place (see AssetSlot<T>). A default-constructed Resource is
     * "empty" (is_valid() == false). AssetManager is the only thing that hands out non-empty
     * ones.
     *
     * @warning A raw `T*`/`T&` obtained from get()/operator->()/operator*() is only guaranteed
     * valid until the next time this specific asset is reloaded or unloaded. Don't hold on to
     * one across a point where that could happen. Call get()/operator->() fresh each time you
     * need it, or use get_shared() to pin the current version for as long as you need it.
     */
    template<typename T>
    class Resource {
    public:
        Resource() = default;
        explicit Resource(std::shared_ptr<AssetSlot<T>> slot) : m_slot(std::move(slot)) {}

        /**
         * @brief True if this handle refers to a slot that currently holds a loaded value.
         */
        bool is_valid() const { return m_slot && m_slot->get() != nullptr; }
        explicit operator bool() const { return is_valid(); }

        /**
         * @brief Gets a shared_ptr to the current value, keeping it alive for as long as the
         * caller holds the returned pointer - even across a subsequent reload/unload.
         */
        std::shared_ptr<T> get_shared() const { return m_slot ? m_slot->get() : nullptr; }

        // See the class-level warning about raw pointer/reference lifetime.
        T* get() const { return get_shared().get(); }
        T& operator*() const { return *get_shared(); }
        T* operator->() const { return get_shared().get(); }

    private:
        std::shared_ptr<AssetSlot<T>> m_slot;
    };
}
