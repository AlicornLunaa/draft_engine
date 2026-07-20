#pragma once

#include <filesystem>

namespace Draft {
    /**
     * @brief A project root on disk. Pure path bookkeeping, no CMake invocation of its own.
     *
     * Everything project-level lives in one file, <root>/manifest.json
     */
    class EditorProject {
    public:
        explicit EditorProject(std::filesystem::path root);

        const std::filesystem::path& root() const { return m_root; }

        /**
         * @brief <root>/assets, or manifest.json's "assetsDir" entry under root if set.
         */
        std::filesystem::path assets_dir() const;

        /**
         * @brief Path to the project's single settings/manifest file.
         */
        std::filesystem::path manifest_path() const;

        /**
         * @brief Resolves the manifest's "module" entry to an absolute path to the compiled
         * game module.
         * @throws std::runtime_error if the manifest doesn't exist yet or has no "module" entry,
         * meaning the project hasn't been built.
         */
        std::filesystem::path resolved_module_path() const;

    private:
        std::filesystem::path m_root;
    };
}
