#pragma once

#include <functional>
#include <vector>
#include <string>

namespace Clydesdale {
    using ConsoleArgs = const std::vector<std::string>&;
    using ConsoleFunc = std::function<void(ConsoleArgs)>;

    class Console {
    private:
        // Constants
        constexpr static size_t INPUT_SIZE = 512;
        constexpr static size_t OUTPUT_SIZE = 4096;
        constexpr static size_t LINE_WIDTH = 64;

        // Variables
        std::vector<ConsoleFunc> commandArray;
        std::vector<std::string> commandAliases;
        char inputBuffer[INPUT_SIZE];
        char outputBuffer[OUTPUT_SIZE];
        unsigned int cursor = 0;

        bool mOpened = false;
        bool mKeyPressed = false; // TODO: Remove with custom input system

        // Private functions
        void parseArguments(const std::string& text, std::vector<std::string>& args);
        std::string addText(const std::string& text);

    public:
        // Constructors
        Console(bool openByDefault = false);
        ~Console();

        // Functions
        void draw();
        void registerCmd(const std::string& key, ConsoleFunc func);
        void deleteCmd(const std::string& key);
        void print(const std::string& text);

        template<typename... Args>
        bool run(std::string key, Args... args){
            // Find the index
            int index = -1;

            for(int i = 0; i < commandAliases.size(); i++){
                if(commandAliases[i] == key){
                    index = i;
                    break;
                }
            }

            // Run command
            if(index != -1){
                auto& func = commandArray[index];
                func({ args... });
                addText("Ran command: " + key);
                return true;
            }

            return false;
        }

        inline bool isOpened(){ return mOpened; }
    };
}