#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

namespace Clydesdale::Widgets {
    class Console {
    private:
        // Constants
        constexpr static size_t INPUT_SIZE = 512;
        constexpr static size_t OUTPUT_SIZE = 4096;
        constexpr static size_t LINE_WIDTH = 64;

        // Variables
        std::unordered_map<std::string, std::function<void(void)>> commands;
        char inputBuffer[INPUT_SIZE];
        char outputBuffer[OUTPUT_SIZE];
        unsigned int cursor = 0;

        // Private functions
        std::string addText(const std::string& txt);

    public:
        // Constructors
        Console();
        ~Console();

        // Functions
        void draw();
        void registerCmd(std::string key, std::function<void(void)> func);
        void deleteCmd(std::string key);
        void print(std::string txt);
        bool run(std::string key);
    };
}