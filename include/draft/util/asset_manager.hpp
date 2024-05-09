#pragma once

#include "draft/util/file_handle.hpp"

namespace Draft {
    // Owns and manages all resources inside the game
    namespace Assets {
        // Functions
        size_t start_package();
        void select_package(size_t package);
        void end_package(size_t package);
        void end_package();

        template<typename T>
        const T& get_asset(const FileHandle& handle);

        template<typename T>
        const T& get_missing_placeholder();

        void reload();
        void cleanup();
    };

    class AssetManager {
    private:
        size_t packageID;

    public:
        AssetManager() : packageID(Assets::start_package()) {}
        AssetManager(const AssetManager& other) = delete;
        AssetManager& operator=(const AssetManager& other) = delete;
        ~AssetManager(){ Assets::end_package(packageID); }

        template<typename T>
        void queue(const FileHandle& handle){
            Assets::select_package(packageID);
            Assets::get_asset<T>(handle);
        }
    };
}