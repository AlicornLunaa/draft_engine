#include "draft/util/logger.hpp"

#include <iostream>
#include <unordered_map>

namespace Draft {
    namespace {
        constexpr const char* ANSI_RESET    = "\033[0m";
        constexpr const char* ANSI_CYAN     = "\033[36m";
        constexpr const char* ANSI_YELLOW   = "\033[33m";
        constexpr const char* ANSI_RED      = "\033[31m";
        constexpr const char* ANSI_BOLD_RED = "\033[1m\033[31m";

        std::unordered_map<std::size_t, Logger::Sink>& sinks(){
            static std::unordered_map<std::size_t, Logger::Sink> instance;
            return instance;
        }

        std::size_t& next_sink_token(){
            static std::size_t instance = 0;
            return instance;
        }
    }

    void Logger::print(LogLevel level, const std::string& name, const std::string& str){
        switch(level){
            case LogLevel::Critical:
                std::cout << ANSI_BOLD_RED << "[!CRITICAL!] [" << name << "] " << str << ANSI_RESET;
                std::cerr << str;
                break;

            case LogLevel::Severe:
                std::cout << ANSI_RED << "[SEVERE] [" << name << "] " << ANSI_RESET << str;
                std::cerr << str;
                break;

            case LogLevel::Warning:
                std::cout << ANSI_YELLOW << "[WARNING] [" << name << "] " << ANSI_RESET << str;
                std::cerr << str;
                break;

            case LogLevel::Info:
                std::cout << ANSI_CYAN << "[INFO] [" << name << "] " << ANSI_RESET << str;
                break;
        }

        for(auto& [token, sink] : sinks())
            sink(level, name, str);
    }

    void Logger::println(LogLevel level, const std::string& name, const std::string& str){
        print(level, name, str + "\n");
    }

    void Logger::print_raw(const std::string& str){
        std::cout << str;
    }

    std::size_t Logger::add_sink(Sink sink){
        std::size_t token = next_sink_token()++;
        sinks()[token] = std::move(sink);
        return token;
    }

    void Logger::remove_sink(std::size_t token){
        sinks().erase(token);
    }
}
