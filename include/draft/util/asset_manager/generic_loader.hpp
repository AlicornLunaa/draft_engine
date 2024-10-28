#pragma once

#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/logger.hpp"

namespace Draft {
    template<typename T>
    struct GenericLoader : public BaseLoader {
        // Variables
        std::vector<std::byte> rawData;

        // Loading on main thread
        virtual std::shared_ptr<void> load_sync() const override {
            // Default to basic call of default filehandle constructor
            try {
                auto ptr = std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });;
                return ptr;
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return nullptr;
        }

        // Loading in a separate thread with no OpenGL context
        virtual void load_async() override {
            rawData = handle.read_bytes();
        }

        // Create the finalized object
        virtual std::shared_ptr<void> finish_async_gl() override {
            // Default to basic call of default filehandle constructor
            try {
                return std::shared_ptr<T>(new T(rawData), [](void* ptr){ delete static_cast<T*>(ptr); });
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return nullptr;
        }

        // Cloning
        virtual BaseLoader* clone(const FileHandle& handle) const override {
            auto* ptr = new GenericLoader<T>();
            ptr->handle = handle;
            return ptr;
        }
    };

    template<typename T>
    struct GenericSyncLoader : public BaseLoader {
        // Loading on main thread
        virtual std::shared_ptr<void> load_sync() const override {
            // Default to basic call of default filehandle constructor
            try {
                return std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return nullptr;
        }

        // Loading in a separate thread with no OpenGL context
        virtual void load_async() override {}

        // Create the finalized object
        virtual std::shared_ptr<void> finish_async_gl() override {
            // Default to basic call of default filehandle constructor
            try {
                return std::shared_ptr<T>(new T(handle), [](void* ptr){ delete static_cast<T*>(ptr); });
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return nullptr;
        }

        // Cloning
        virtual BaseLoader* clone(const FileHandle& handle) const override {
            auto* ptr = new GenericSyncLoader<T>();
            ptr->handle = handle;
            return ptr;
        }
    };
};