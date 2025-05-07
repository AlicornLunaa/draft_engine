#pragma once

#include "draft/util/asset_manager/asset_ptr.hpp"
#include "draft/util/asset_manager/base_loader.hpp"
#include "draft/util/logger.hpp"
#include <memory>

namespace Draft {
    template<typename T>
    struct GenericLoader : public BaseLoader {
        // Variables
        std::vector<std::byte> rawData;

        // Constructors
        GenericLoader() : BaseLoader(typeid(T)) {}

        // Loading on main thread
        virtual AssetPtr load_sync(Assets& assets) const override {
            // Default to basic call of default filehandle constructor
            try {
                return make_asset_ptr(new T(handle));
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return make_asset_ptr<T>(nullptr);
        }

        // Loading in a separate thread with no OpenGL context
        virtual void load_async() override {
            rawData = handle.read_bytes();
        }

        // Create the finalized object
        virtual AssetPtr finish_async_gl(Assets& assets) override {
            // Default to basic call of default filehandle constructor
            try {
                return make_asset_ptr(new T(rawData));
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return make_asset_ptr<T>(nullptr);
        }

        // Cloning
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override {
            auto ptr = std::unique_ptr<BaseLoader>(new GenericLoader<T>());
            ptr->handle = handle;
            return ptr;
        }
    };

    template<typename T>
    struct GenericSyncLoader : public BaseLoader {
        // Constructors
        GenericSyncLoader() : BaseLoader(typeid(T)) {}

        // Loading on main thread
        virtual AssetPtr load_sync(Assets& assets) const override {
            // Default to basic call of default filehandle constructor
            try {
                return make_asset_ptr(new T(handle));
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return make_asset_ptr<T>(nullptr);
        }

        // Loading in a separate thread with no OpenGL context
        virtual void load_async() override {}

        // Create the finalized object
        virtual AssetPtr finish_async_gl(Assets& assets) override {
            // Default to basic call of default filehandle constructor
            try {
                return make_asset_ptr(new T(handle));
            } catch(int e){
                Logger::print(Level::SEVERE, typeid(T).name(), std::to_string(e));
            }

            return make_asset_ptr<T>(nullptr);
        }

        // Cloning
        virtual std::unique_ptr<BaseLoader> clone(const FileHandle& handle) const override {
            auto ptr = std::unique_ptr<BaseLoader>(new GenericSyncLoader<T>());
            ptr->handle = handle;
            return ptr;
        }
    };
};