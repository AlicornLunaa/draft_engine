#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"

#include <cassert>

namespace Draft {
    class Assets;

    template<typename T>
    class Resource {
    private:
        // Variables
        AssetPtr* ptr; // Reference to the unique ptr of the resource

        // Private constructors because this shouldnt be made by the end-user
        Resource(AssetPtr& ptr) : ptr(&ptr) {};

    public:
        // Constructors
        Resource(const Resource<T>& other) : ptr(other.ptr) {};
        ~Resource() = default;

        // Freinds :)
        friend class Assets;

        // Functions
        T* get_ptr(){ return static_cast<T*>(ptr->get()); }
        const T* get_ptr() const { return static_cast<T*>(ptr->get()); }
        T& get(){ return *get_ptr(); }
        const T& get() const { return *get_ptr(); }

        // Operators
        Resource<T>& operator=(const Resource<T>& other){
            ptr = other.ptr;
            return *this;
        }

        operator T*() { return get_ptr(); }
        operator T&() { return get(); }
        operator const T&() const { return get(); }
    };
};