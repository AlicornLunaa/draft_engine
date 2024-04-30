#pragma once

#include <functional>
#include <sstream>
#include <vector>
#include <string>

namespace Draft {
    using ConsoleArgs = const std::vector<std::string>&;
    using ConsoleFunc = std::function<bool(ConsoleArgs)>;

    class Application;

    class Console {
    private:
        // Constants
        constexpr static size_t INPUT_BUFFER_SIZE = 512;
        constexpr static size_t MAX_LINES = 100;

        // Variables
        const Application* app;
        std::streambuf* oldOutBuf = nullptr;
        std::vector<ConsoleFunc> commandArray;
        std::vector<std::string> commandAliases;

        std::vector<std::string> lines;
        std::ostringstream stream;

        char inputBuffer[INPUT_BUFFER_SIZE]{'\0'};
        bool mOpened = false;
        bool scrollToBottom = true;
        bool reclaimFocus = true;

        // Private functions
        void parse_arguments(const std::string& text, std::vector<std::string>& args);

    public:
        // Constructors
        Console(const Application* app, bool openByDefault = false);
        ~Console();

        // Functions
        void draw();
        void set_open(bool open = true);
        void register_cmd(const std::string& key, ConsoleFunc func);
        void delete_cmd(const std::string& key);
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

        inline bool is_opened(){ return mOpened; }
        inline std::ostringstream& get_stream(){ return stream; }
    };
}