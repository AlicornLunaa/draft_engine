#include <iostream>
#include "logger.hpp"
#include "ansi_colors.hpp"

namespace Clydesdale {
    void Logger::print(const Level level, const std::string& name, const std::string& str){
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
    }

    void Logger::println(const Level level, const std::string& name, const std::string& str){
        print(level, name, str + "\n");
    }

    void Logger::printRaw(const std::string& str){
        std::cout << str;
    }
}