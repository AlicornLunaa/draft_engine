#include "draft/util/files/embedded_file_provider.hpp"

#include "cmrc/cmrc.hpp"

#include <stdexcept>

CMRC_DECLARE(draft_engine);

namespace fs = std::filesystem;

namespace Draft {
    std::unique_ptr<FileProvider> EmbeddedFileProvider::clone() const {
        return std::make_unique<EmbeddedFileProvider>(*this);
    }

    bool EmbeddedFileProvider::exists(const fs::path& path) const {
        return cmrc::draft_engine::get_filesystem().exists(path.string());
    }

    bool EmbeddedFileProvider::is_directory(const fs::path& path) const {
        return cmrc::draft_engine::get_filesystem().is_directory(path.string());
    }

    bool EmbeddedFileProvider::is_file(const fs::path& path) const {
        return cmrc::draft_engine::get_filesystem().is_file(path.string());
    }

    std::uintmax_t EmbeddedFileProvider::size(const fs::path& path) const {
        // cmrc::open() throws for a missing resource; let that propagate
        return cmrc::draft_engine::get_filesystem().open(path.string()).size();
    }

    Time EmbeddedFileProvider::last_modified(const fs::path&) const {
        // cmrc bakes resources into the binary at build time and exposes no timestamp metadata;
        // this is a documented limitation, not a failure, so it doesn't throw.
        return Time::microseconds(0);
    }

    bool EmbeddedFileProvider::remove(const fs::path& path) const {
        throw std::logic_error("EmbeddedFileProvider: cannot remove '" + path.string() + "', embedded resources are read-only");
    }

    bool EmbeddedFileProvider::create_directories(const fs::path& path) const {
        throw std::logic_error("EmbeddedFileProvider: cannot create directories for '" + path.string() + "', embedded resources are read-only");
    }

    std::string EmbeddedFileProvider::read_string(const fs::path& path) const {
        auto file = cmrc::draft_engine::get_filesystem().open(path.string());
        return std::string(file.begin(), file.end());
    }

    void EmbeddedFileProvider::write_string(const fs::path& path, const std::string&) const {
        throw std::logic_error("EmbeddedFileProvider: cannot write to '" + path.string() + "', embedded resources are read-only");
    }

    std::vector<std::byte> EmbeddedFileProvider::read_bytes(const fs::path& path, std::size_t offset) const {
        auto file = cmrc::draft_engine::get_filesystem().open(path.string());
        if (offset >= file.size()) return {};

        std::vector<std::byte> buffer;
        buffer.reserve(file.size() - offset);

        auto it = file.begin();
        std::advance(it, offset);

        for (; it != file.end(); ++it) {
            buffer.push_back(static_cast<std::byte>(*it));
        }
        return buffer;
    }

    void EmbeddedFileProvider::write_bytes(const fs::path& path, const void*, std::size_t) const {
        throw std::logic_error("EmbeddedFileProvider: cannot write to '" + path.string() + "', embedded resources are read-only");
    }

    std::vector<fs::path> EmbeddedFileProvider::list(const fs::path& path) const {
        if (!is_directory(path)) return {};

        std::vector<fs::path> result;
        for (const auto& entry : cmrc::draft_engine::get_filesystem().iterate_directory(path.string())) {
            result.push_back(path / entry.filename());
        }
        return result;
    }

    std::string EmbeddedFileProvider::get_absolute_path(const fs::path& path) const {
        // Embedded resources have no real filesystem location, so the relative path is as
        // "absolute" as it gets.
        return path.string();
    }
}
