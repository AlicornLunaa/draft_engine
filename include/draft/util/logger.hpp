#pragma once

#include <string>

namespace Draft {
    enum class Level { INFO, WARNING, SEVERE, CRITICAL };
    
    namespace Logger {
        void print(const Level level, const std::string& name, const std::string& str);
        void println(const Level level, const std::string& name, const std::string& str);
        void print_raw(const std::string& str);
    }
}