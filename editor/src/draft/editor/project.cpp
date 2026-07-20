#include "draft/editor/project.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"

#include <stdexcept>

namespace Draft {
    EditorProject::EditorProject(std::filesystem::path root) : m_root(std::filesystem::absolute(root)) {}

    std::filesystem::path EditorProject::assets_dir() const {
        return m_root / "assets";
    }

    std::filesystem::path EditorProject::manifest_path() const {
        return m_root / "manifest.json";
    }

    std::filesystem::path EditorProject::module_manifest_path() const {
        return m_root / "build" / "bin" / "game.json";
    }

    std::filesystem::path EditorProject::resolved_module_path() const {
        std::filesystem::path manifestPath = module_manifest_path();
        FileHandle manifest = HostFileSystem().open(manifestPath);

        if(!manifest.exists())
            throw std::runtime_error("EditorProject::resolved_module_path(): no build found at " + manifestPath.string());

        JSON json(manifest);
        std::string relative = json.value("module", std::string());
        return std::filesystem::absolute(manifestPath.parent_path() / relative);
    }
}
