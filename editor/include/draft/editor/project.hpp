#pragma once

#include <filesystem>

namespace Draft {
    /**
     * @brief A project root on disk. Pure path bookkeeping, no CMake invocation of its own.
     */
    class EditorProject {
    public:
        explicit EditorProject(std::filesystem::path root);

        const std::filesystem::path& root() const { return m_root; }
        std::filesystem::path assets_dir() const;

        /**
         * @brief Path to the project's own root-level settings file (editor preferences today,
         * a natural home for module/asset location too if that ever moves out of the
         * build-generated module manifest below). Editor-owned, not written by CMake.
         */
        std::filesystem::path manifest_path() const;

        /**
         * @brief Path to the manifest draft_launcher itself reads, written at build time by
         * draft_add_game_module (see build_tools/CMakeLists.txt).
         */
        std::filesystem::path module_manifest_path() const;

        /**
         * @brief Resolves the manifest's "module" entry to an absolute path to the compiled
         * game module.
         * @throws std::runtime_error if the manifest doesn't exist yet, meaning the project
         * hasn't been built.
         */
        std::filesystem::path resolved_module_path() const;

    private:
        std::filesystem::path m_root;
    };
}
