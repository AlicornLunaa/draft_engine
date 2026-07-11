#pragma once

#include <string>

namespace Draft {
    /**
     * @brief Severity of a Logger message.
     *
     * Controls both the ANSI color a message is printed in and whether it also gets
     * duplicated to stderr.
     */
    enum class LogLevel {
        Info,
        Warning,
        Severe,
        Critical,
    };

    /**
     * @brief Minimal stdout/stderr logger with per-level ANSI coloring.
     */
    namespace Logger {
        /**
         * @brief Writes @p str to stdout, colored and tagged with @p level and @p name.
         *
         * Warning and above are also duplicated, uncolored, to stderr.
         */
        void print(LogLevel level, const std::string& name, const std::string& str);

        /**
         * @brief Same as print(), with a trailing newline appended to @p str.
         */
        void println(LogLevel level, const std::string& name, const std::string& str);

        /**
         * @brief Writes @p str to stdout as-is, with no level tag, name, or coloring.
         */
        void print_raw(const std::string& str);
    }
}
