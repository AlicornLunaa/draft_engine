#pragma once

#include <memory>

namespace Draft {
    // Implementation of a type-erased standard unique_ptr
    using AssetPtr = std::unique_ptr<void, void(*)(void*)>;

    template<typename T>
    AssetPtr make_asset_ptr(T* ptr){
        return AssetPtr(ptr, [](void* rawPtr){ delete static_cast<T*>(rawPtr); });
    }
};