#pragma once

#include "draft/util/clock.hpp"
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
         * @brief True once the watched file's mtime has changed from what it was at construction.
         */
        bool poll();

    private:
        std::filesystem::path m_path;
        std::optional<Time> m_lastModified;

        std::optional<Time> m_pendingModified;
        Clock m_pendingClock;
    };
}
