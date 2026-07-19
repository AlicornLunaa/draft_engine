#pragma once

#include <filesystem>

namespace Draft {
    /**
     * @brief Validates every asset under <projectRoot>/assets (see validate_assets(),
     * asset_pipeline.hpp), resaves them into a temp directory. Scene JSON normalized via
     * JSON::parse()/dump(), everything else copied byte for byte and zips the result into a
     * single .apak at @p outputPath via ApakWriter. Owns its own hidden GL context for the
     * duration of the call (Texture/Font/Model validation issues real GL calls) and leaves the
     * calling thread's current_path() unchanged on return.
     *
     * Progress and any per-asset failures are logged through Logger as they happen, the same
     * way whether this is called from the CLI or (later) an editor
     *
     * @return False if any asset failed validation (already logged; nothing written to
     * @p outputPath in that case). True once @p outputPath has been written.
     * @throws std::runtime_error if resaving or writing the archive itself fails (a real I/O
     * problem, not an invalid asset).
     */
    bool pack_project(const std::filesystem::path& projectRoot, const std::filesystem::path& outputPath);
}
