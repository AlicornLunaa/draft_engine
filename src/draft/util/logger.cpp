#include <iostream>
#include "draft/util/logger.hpp"
#include "draft/util/ansi_colors.hpp"

namespace Draft {
    void Logger::print(const Level level, const std::string& name, const std::string& str){
        #ifdef ANSI_SUPPORT
        switch(level){
            case Level::CRITICAL:
                std::cout << Color::BoldRed << "[!CRITICAL!] " << "[" << name << "] " << str << Color::Reset;
                break;
                
            case Level::SEVERE:
                std::cout << Color::Red << "[SEVERE] " << "[" << name << "] " << Color::Reset << str;
                break;
                
            case Level::WARNING:
                std::cout << Color::Yellow << "[WARNING] " << "[" << name << "] " << Color::Reset << str;
                break;

            default:
                std::cout << Color::Cyan << "[INFO] " << "[" << name << "] " << Color::Reset << str;
                break;
        }
        #else
        switch(level){
            case Level::CRITICAL:
                std::cout << "[!CRITICAL!] " << "[" << name << "] " << str;
                break;
                
            case Level::SEVERE:
                std::cout << "[SEVERE] " << "[" << name << "] " << str;
                break;
                
            case Level::WARNING:
                std::cout << "[WARNING] " << "[" << name << "] " << str;
                break;

            default:
                std::cout << "[INFO] " << "[" << name << "] " << str;
                break;
        }
        #endif
    }

    void Logger::println(const Level level, const std::string& name, const std::string& str){
        print(level, name, str + "\n");
    }

    void Logger::print_raw(const std::string& str){
        std::cout << str;
    }
}