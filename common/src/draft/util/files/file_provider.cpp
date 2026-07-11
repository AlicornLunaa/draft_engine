#include "draft/util/files/file_provider.hpp"
#include "draft/util/files/file_handle.hpp"

namespace Draft {
    // Defined here because FileHandle's definition isn't visible yet at the point FileProvider is declared.
    // The two headers only forward-declare each other to avoid a circular include.
    FileHandle FileProvider::open(const std::filesystem::path& path) const {
        return FileHandle(path, *this);
    }
}
