#pragma once

#include <filesystem>
#include <string>

namespace Draft {
    /**
     * @brief Merges "module": @p moduleRelativePath into <projectRoot>/manifest.json, creating
     * it if it doesn't exist yet. Every other top-level key already there (e.g. the "editor"
     * block EditorApplication::save_settings() writes) is preserved untouched.
     */
    void write_module_manifest(const std::filesystem::path& projectRoot, const std::string& moduleRelativePath);
}
