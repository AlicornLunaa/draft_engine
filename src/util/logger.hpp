#pragma once

#include <string>
#include "ansi_colors.hpp"

namespace SpaceGame {
    namespace Util {
        namespace Logger {
            enum class Level { INFO, WARNING, SEVERE, CRITICAL };
            void print(const Level level, const std::string& name, const std::string& str);
            void println(const Level level, const std::string& name, const std::string& str);
            void printRaw(const std::string& str);
        }
    }
}