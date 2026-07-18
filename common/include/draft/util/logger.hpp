#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>

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

        /**
         * @brief A callback print() invokes for every message, alongside its own stdout/stderr
         * writes. Doesn't see print_raw() calls.
         */
        using Sink = std::function<void(LogLevel level, std::string_view name, std::string_view message)>;

        /**
         * @brief Registers @p sink. Returns a token remove_sink() can use to unregister it.
         */
        std::size_t add_sink(Sink sink);

        /**
         * @brief Unregisters a sink previously returned by add_sink(). No-op if @p token isn't
         * currently registered.
         */
        void remove_sink(std::size_t token);
    }
}
