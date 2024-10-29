#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/file_handle.hpp"
#include <typeindex>

namespace Draft {
    struct BaseLoader {
        // Variables
        const std::type_index type;
        FileHandle handle;

        // Constructors
        BaseLoader(std::type_index type) : type(type) {};
        virtual ~BaseLoader() = default;

        // Functions
        virtual AssetPtr load_sync() const = 0; // Loading on main thread

        virtual void load_async() = 0; // Loading in a separate thread with no OpenGL context
        virtual AssetPtr finish_async_gl() = 0; // Called after async loading without OpenGL

        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const = 0;
    };
};