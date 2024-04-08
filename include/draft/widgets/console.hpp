#pragma once

#include "draft/util/circular_buffer.hpp"

#include <functional>
#include <sstream>
#include <vector>
#include <string>

namespace Draft {
    using ConsoleArgs = const std::vector<std::string>&;
    using ConsoleFunc = std::function<bool(ConsoleArgs)>;

    class Console {
    private:
        // Constants
        constexpr static size_t INPUT_BUFFER_SIZE = 512;
        constexpr static size_t OUTPUT_BUFFER_SIZE = 4096;
        constexpr static size_t MAX_LINES = 32;

        // Variables
        std::vector<ConsoleFunc> commandArray;
        std::vector<std::string> commandAliases;

        CircularBuffer<std::string> output = CircularBuffer<std::string>(MAX_LINES);
        std::ostringstream stream;
        size_t lineWidth = 64;

        char inputBuffer[INPUT_BUFFER_SIZE];
        char outputBuffer[OUTPUT_BUFFER_SIZE];

        bool mOpened = false;
        bool mKeyPressed = false; // TODO: Remove with custom input system

        // Private functions
        void parseArguments(const std::string& text, std::vector<std::string>& args);
        void constructRawBuffer();

    public:
        // Constructors
        Console(bool openByDefault = false);
        ~Console();

        // Functions
        void draw();
        void set_open(bool open = true);
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
                bool res;

                try {
                    res = func({ args... });
                } catch(...){
                    res = false;
                }
                
                if(res)
                    print("Ran command: " + key + "\n");

                return res;
            }

            return false;
        }

        inline bool isOpened(){ return mOpened; }
        inline std::ostringstream& getStream(){ return stream; }
    };
}