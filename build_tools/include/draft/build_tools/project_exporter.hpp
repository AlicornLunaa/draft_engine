#pragma once

#include <filesystem>

namespace Draft {
    /**
     * @brief Configures and builds @p projectRoot in Release (producing a self-contained,
     * statically-linked launcher executable whose install() rule this relies on to find the
     * result generically, without needing to know the project's own target name), installs
     * that binary into @p outputDir, and packs the project's assets into
     * <outputDir>/<project-name>.apak via pack_project(). Uses a scratch build directory under
     * @p outputDir, removed again once the install step has copied out what it needs.
     *
     * @throws std::runtime_error if @p projectRoot isn't a directory, if any of the
     * configure/build/install steps (shelled out to a real `cmake` invocation) exits non-zero,
     * or if asset packing fails.
     */
    void export_project(const std::filesystem::path& projectRoot, const std::filesystem::path& outputDir);
}
