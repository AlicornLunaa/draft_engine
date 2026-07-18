#pragma once

#include "draft/util/time.hpp"

#include <filesystem>
#include <optional>

namespace Draft {
    /**
     * @brief Watches one file's modification time. Meant to be polled once per frame or so,
     * not treated as a live filesystem event source.
     */
    class GameModuleWatcher {
    public:
        explicit GameModuleWatcher(std::filesystem::path path);

        /**
         * @brief True once, the first time the watched file's mtime differs from what it was
         * at construction (or the last time this returned true). False if the file doesn't
         * currently exist.
         */
        bool poll();

    private:
        std::filesystem::path m_path;
        std::optional<Time> m_lastModified;
    };
}
