#pragma once

#include "draft/util/file_handle.hpp"

namespace Draft {
    struct BaseLoader {
        FileHandle handle;
        virtual ~BaseLoader(){}

        virtual std::shared_ptr<void> load_sync() const = 0; // Loading on main thread

        virtual void load_async() = 0; // Loading in a separate thread with no OpenGL context
        virtual std::shared_ptr<void> finish_async_gl() = 0; // Called after async loading without OpenGL

        virtual BaseLoader* clone(const FileHandle& handle) const = 0;
    };
};