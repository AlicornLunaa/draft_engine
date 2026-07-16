#pragma once

#include <filesystem>

namespace Draft {
    namespace ApakWriter {
        /**
         * @brief Zips every regular file under @p sourceDir (recursed) into a new archive at
         * @p outputPath. Archive-internal entry names are @p sourceDir-relative and
         * forward-slashed, matching AssetFileSystem's own "assets/..." key shape.
         * @throws std::runtime_error if @p sourceDir isn't a directory, or if writing fails.
         */
        void write(const std::filesystem::path& sourceDir, const std::filesystem::path& outputPath);
    };
}
