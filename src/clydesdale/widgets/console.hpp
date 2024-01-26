#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
#include <string>

namespace Clydesdale::Widgets {
    class Console {
    private:
        // Static variables
        static char inputBuffer[512];

        // Variables
        std::unordered_map<std::string, std::function<void(void)>> commands;
        std::vector<std::string> logQueue;

        // Private functions
        void addText(const std::string& txt);

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