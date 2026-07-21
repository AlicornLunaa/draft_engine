#pragma once

namespace Draft {
    /**
     * @brief Installs OS-level crash handling, segfault/illegal-instruction/abort/... signals on
     * Linux (on their own alternate stack, so a stack-overflow crash can still be handled), an
     * unhandled structured-exception filter on Windows, plus a std::terminate handler for an
     * uncaught C++ exception.
     *
     * On crash, writes a best-effort report (reason, timestamp, and a backtrace where the
     * platform can produce one) to "crashes/crash_<timestamp>.log" relative to the current
     * working directory, then lets the OS's own default crash behavior (core dump, Windows
     * Error Reporting, the process's normal non-zero exit code, ...) still happen afterward.
     */
    namespace CrashHandler {
        void install();
    }
}
