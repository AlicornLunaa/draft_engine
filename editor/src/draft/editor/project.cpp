#include "draft/editor/project.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"

#include <stdexcept>

namespace Draft {
    EditorProject::EditorProject(std::filesystem::path root) : m_root(std::filesystem::absolute(root)) {}

    std::filesystem::path EditorProject::assets_dir() const {
        FileHandle manifest = HostFileSystem().open(manifest_path());
        if(manifest.exists()){
            JSON json(manifest);
            std::string relative = json.value("assetsDir", std::string());
            if(!relative.empty())
                return m_root / relative;
        }

        return m_root / "assets";
    }

    std::filesystem::path EditorProject::manifest_path() const {
        return m_root / "manifest.json";
    }

    std::filesystem::path EditorProject::resolved_module_path() const {
        std::filesystem::path manifestPath = manifest_path();
        FileHandle manifest = HostFileSystem().open(manifestPath);

        if(!manifest.exists())
            throw std::runtime_error("EditorProject::resolved_module_path(): no manifest found at " + manifestPath.string() + " - build the project first");

        JSON json(manifest);
        std::string relative = json.value("module", std::string());
        if(relative.empty())
            throw std::runtime_error("EditorProject::resolved_module_path(): " + manifestPath.string() + " has no \"module\" entry - build the project first");

        return std::filesystem::absolute(m_root / relative);
    }
}
