#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"

#include <cassert>

namespace Draft {
    class Assets;

    template<typename T>
    class Resource {
    private:
        // Variables
        AssetPtr* ptr = nullptr; // Reference to the unique ptr of the resource
        T* dummyPtr = nullptr; // Used exclusively for construction from direct reference.

        // Private constructors because this shouldnt be made by the end-user
        Resource(AssetPtr& ptr) : ptr(&ptr) {};

    public:
        // Constructors
        Resource(T& ref) : dummyPtr(&ref) {}; // Used for implicit conversions
        Resource(T* p) : dummyPtr(p) { assert(p && "Cannot be null"); }; // Used for implicit conversions
        Resource(const Resource<T>& other) : ptr(other.ptr), dummyPtr(other.dummyPtr) {};
        ~Resource() = default;

        // Freinds :)
        friend class Assets;

        // Functions
        bool is_redirecting() const { return dummyPtr; } // Returns true if the resource was not actually made by the manager, acting as a redirect only
        T* get_ptr(){ return static_cast<T*>(ptr ? ptr->get() : dummyPtr); }
        T& get(){ return *get_ptr(); }
        const T* get_ptr() const { return static_cast<T*>(ptr ? ptr->get() : dummyPtr); }
        const T& get() const { return *get_ptr(); }

        // Operators
        Resource<T>& operator=(const Resource<T>& other){
            ptr = other.ptr;
            dummyPtr = other.dummyPtr;
            return *this;
        }

        T& operator*(){ return *get_ptr(); }
        const T& operator*() const { return *get_ptr(); }

        operator T*() { return get_ptr(); }
        operator T&() { return get(); }
        operator const T*() const { return get_ptr(); }
        operator const T&() const { return get(); }
    };
};