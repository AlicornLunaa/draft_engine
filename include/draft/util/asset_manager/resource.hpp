#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"

#include <cassert>
#include <functional>
#include <utility>

namespace Draft {
    class Assets;

    /// Used ONLY for debug and other bullshit static defaulted resources, anything else will because leaks.
    template<typename T>
    class StaticResource {
    private:
        // Variables
        T* m_ptr = nullptr;
        std::function<void(void)> m_construct;

    public:
        // Constructors
        template<typename... Args>
        StaticResource(Args&&... args){
            // Create a lambda expression to construct T from args
            m_construct = [this, ...args = std::forward<Args>(args)]() mutable {
                m_ptr = new T(std::move(args)...);
            };
        }

        // Functions
        T* get_ptr(){
            if(!m_ptr)
                m_construct();

            return m_ptr;
        }

        T& get(){ return *get_ptr(); }
        const T* get_ptr() const { return get_ptr(); }
        const T& get() const { return *get_ptr(); }

        bool is_valid() const { return m_ptr != nullptr; }

        // Operators
        StaticResource<T>& operator=(const StaticResource<T>& other) = delete;

        T* operator->(){ return get_ptr(); }
        const T* operator->() const { return get_ptr(); }
        T& operator*(){ return *get_ptr(); }
        const T& operator*() const { return *get_ptr(); }

        operator T*() { return get_ptr(); }
        operator T&() { return get(); }
        operator const T*() const { return get_ptr(); }
        operator const T&() const { return get(); }
    };

    template<typename T>
    class Resource {
    private:
        // Variables
        AssetPtr* ptr = nullptr; // Reference to the unique ptr of the resource
        T* dummyPtr = nullptr; // Used exclusively for construction from direct reference.

        // Private constructors because this shouldnt be made by the end-user
        Resource(AssetPtr& ptr) : ptr(&ptr) {};

        // Private functions
        T* get_dummy_ptr_safe() const {
            assert(dummyPtr && "Cannot be used as null");
            return dummyPtr;
        }

    public:
        // Constructors
        Resource(T& ref) : dummyPtr(&ref) {}; // Used for implicit conversions
        Resource(T* p) : dummyPtr(p) {}; // Used for implicit conversions
        Resource(StaticResource<T>& other) : Resource(other.get()) {};
        Resource(const Resource<T>& other) : ptr(other.ptr), dummyPtr(other.dummyPtr) {};
        ~Resource() = default;

        // Freinds :)
        friend class Assets;

        // Functions
        bool is_valid() const { return dummyPtr != nullptr || ptr != nullptr; }
        bool is_redirecting() const { return dummyPtr; } // Returns true if the resource was not actually made by the manager, acting as a redirect only
        T* get_ptr(){ return static_cast<T*>(ptr ? ptr->get() : get_dummy_ptr_safe()); }
        T& get(){ return *get_ptr(); }
        const T* get_ptr() const { return static_cast<T*>(ptr ? ptr->get() : get_dummy_ptr_safe()); }
        const T& get() const { return *get_ptr(); }

        // Operators
        Resource<T>& operator=(const Resource<T>& other){
            ptr = other.ptr;
            dummyPtr = other.dummyPtr;
            return *this;
        }

        T* operator->(){ return get_ptr(); }
        const T* operator->() const { return get_ptr(); }
        T& operator*(){ return *get_ptr(); }
        const T& operator*() const { return *get_ptr(); }

        operator T*() { return get_ptr(); }
        operator T&() { return get(); }
        operator const T*() const { return get_ptr(); }
        operator const T&() const { return get(); }
    };
};