#include "draft/build_tools/project_manifest.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/files/host_file_system.hpp"
#include "draft/util/json.hpp"

namespace Draft {
    void write_module_manifest(const std::filesystem::path& projectRoot, const std::string& moduleRelativePath){
        FileHandle manifest = HostFileSystem().open(projectRoot / "manifest.json");

        JSON json = manifest.exists() ? JSON(manifest) : JSON::object();
        json["module"] = moduleRelativePath;

        manifest.write_string(json.dump(4));
    }
}
