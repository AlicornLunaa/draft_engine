#include <cstddef>
#include <format>
#include <string>
#include <vector>

#include "draft/widgets/console.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/util/logger.hpp"
#include "imgui.h"

using namespace std;

namespace Draft {
    // Private functions
    void Console::parse_arguments(const string& text, vector<string>& args){
        string token = "";

        auto newToken = [&args, &token](){
            args.push_back(token);
            token = "";
        };

        for(size_t i = 0; i < text.size(); i++){
            char ch = text[i];

            if(ch == ' '){
                newToken();
                continue;
            } else if(ch == '\0') {
                newToken();
                break;
            }

            token += ch;
        }
    }

    void Console::construct_raw_buffer(){
        // Creates a raw buffer from the input buffer
        for(size_t i = 0; i < OUTPUT_BUFFER_SIZE; i++){
            // Zero out the buffer
            outputBuffer[i] = '\0';
        }

        size_t cursor = 0;

        for(size_t i = 0; i < output.length(); i++){
            // Add the line to the buffer
            string line = output[i];
            
            for(const char& c : line){
                if(cursor >= OUTPUT_BUFFER_SIZE) break;
                if(c == '\0') continue;
                outputBuffer[cursor] = c;
                cursor++;
            }
        }
    }

    // Constructors
    Console::Console(bool openByDefault){
        // Open by default flag
        mOpened = openByDefault;

        // Zero out buffers
        for(int i = 0; i < INPUT_BUFFER_SIZE; i++)
            inputBuffer[i] = '\0';
            
        for(int i = 0; i < OUTPUT_BUFFER_SIZE; i++)
            outputBuffer[i] = '\0';
    }

    Console::~Console(){}

    // Functions
    void Console::draw(){
        // Interface
        if(mOpened){
            ImGui::SetNextWindowSizeConstraints({ 300, 200 }, { FLT_MAX, FLT_MAX });
            ImGui::Begin("Console", &mOpened);
            lineWidth = (ImGui::GetWindowWidth() - 20) / 8;

            // Copy stream data to the buffer
            if(stream.str().length() > 0){
                print(stream.str());
                stream.str("");
            }

            // List data
            ImGui::InputTextMultiline("##console_logs", outputBuffer, OUTPUT_BUFFER_SIZE, ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 2 - 16), ImGuiInputTextFlags_ReadOnly);

            // Input handling
            ImGui::SetNextItemWidth(-72);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - 4);
            ImGui::InputTextWithHint("##", "COMMAND", &inputBuffer[0], 512);
            ImGui::SameLine();

            if(ImGui::Button("RUN", { 64, ImGui::GetFrameHeight() }) || (Keyboard::is_just_pressed(Key::ENTER) && inputBuffer[0] != '\0')){
                string rawCommand(inputBuffer, 512);
                vector<string> argList;
                parse_arguments(rawCommand, argList);
                print(rawCommand + '\n');
                run(argList[0], argList);

                for(int i = 0; i < 512; i++)
                    inputBuffer[i] = '\0';
            }

            ImGui::End();
        }

        // Handle pressing keys
        if(Keyboard::is_just_pressed(Key::GRAVE)){
            mOpened = !mOpened;
        }
    }

    void Console::set_open(bool open){
        mOpened = open;
    }

    void Console::register_cmd(const string& key, ConsoleFunc func){
        commandAliases.push_back(key);
        commandArray.push_back(func);
        Logger::println(Level::INFO, "Console", std::format("Command {} registered", key));
    }

    void Console::delete_cmd(const string& key){
        // Find the index
        int index = -1;

        for(int i = 0; i < commandAliases.size(); i++){
            if(commandAliases[i] == key){
                index = i;
                break;
            }
        }

        // Remove from records
        if(index != -1){
            commandAliases.erase(commandAliases.begin() + index);
            commandArray.erase(commandArray.begin() + index);
            Logger::println(Level::INFO, "Console", std::format("Command {} unregistered", key));
        }
    }

    void Console::print(const string& text){
        // Add the text provided to the circular buffer
        output.push(text);
        construct_raw_buffer();
    }
}