#include "draft/build_tools/apak_writer.hpp"

#include "miniz.h"

#include <stdexcept>

namespace fs = std::filesystem;

namespace Draft {
    void ApakWriter::write(const fs::path& sourceDir, const fs::path& outputPath) {
        if (!fs::is_directory(sourceDir))
            throw std::runtime_error("ApakWriter: '" + sourceDir.string() + "' is not a directory");

        mz_zip_archive archive{};
        if (!mz_zip_writer_init_file(&archive, outputPath.string().c_str(), 0))
            throw std::runtime_error("ApakWriter: failed to create '" + outputPath.string() + "'");

        try {
            for (fs::recursive_directory_iterator it(sourceDir), end; it != end; ++it) {
                if (it->is_directory()) continue;

                std::string archiveName = fs::relative(it->path(), sourceDir).generic_string();

                if (!mz_zip_writer_add_file(&archive, archiveName.c_str(), it->path().string().c_str(), nullptr, 0, MZ_DEFAULT_COMPRESSION)) {
                    mz_zip_error err = mz_zip_get_last_error(&archive);
                    throw std::runtime_error("ApakWriter: failed to add '" + archiveName + "' (" + mz_zip_get_error_string(err) + ")");
                }
            }

            if (!mz_zip_writer_finalize_archive(&archive))
                throw std::runtime_error("ApakWriter: failed to finalize '" + outputPath.string() + "'");
        } catch (...) {
            mz_zip_writer_end(&archive); // best-effort cleanup; outputPath may be left partially written
            throw;
        }

        mz_zip_writer_end(&archive);
    }
}
