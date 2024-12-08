#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include <iostream> // IWYU pragma: keep

#include "draft/interface/widgets/console.hpp"
#include "draft/core/application.hpp"
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

    // Constructors
    Console::Console(const Application* app, bool openByDefault) : app(app) {
        // Open by default flag
        mOpened = openByDefault;

        // Zero out buffers
        for(int i = 0; i < INPUT_BUFFER_SIZE; i++)
            inputBuffer[i] = '\0';

        // Redirect cout to console
        oldOutBuf = std::cout.rdbuf(get_stream().rdbuf());
    }

    Console::~Console(){
        // Restore cout to stdout
        std::cout.rdbuf(oldOutBuf);
    }

    // Functions
    void Console::draw(){
        // Lambda
        auto run_command = [this](){
            string rawCommand(inputBuffer, 512);
            vector<string> argList;
            parse_arguments(rawCommand, argList);
            print(rawCommand + '\n');
            run(argList[0], argList);

            for(int i = 0; i < 512; i++)
                inputBuffer[i] = '\0';
        };

        auto get_ansi_color = [](const std::string& str){
            // Gets ImGui color from ANSI color code
            if(str == "\033[0m") return ImVec4(1, 1, 1, 1);
            else if(str == "\033[30m") return ImVec4(0, 0, 0, 1);
            else if(str == "\033[31m") return ImVec4(1, 0, 0, 1);
            else if(str == "\033[32m") return ImVec4(0, 1, 0, 1);
            else if(str == "\033[33m") return ImVec4(1, 1, 0, 1);
            else if(str == "\033[34m") return ImVec4(0, 0, 1, 1);
            else if(str == "\033[35m") return ImVec4(1, 0, 1, 1);
            else if(str == "\033[36m") return ImVec4(0, 1, 1, 1);
            else if(str == "\033[37m") return ImVec4(1, 1, 1, 1);
            return ImVec4(1, 1, 1, 1);
        };

        // Interface
        if(mOpened){
            ImGui::SetNextWindowSizeConstraints({ 480, 270 }, { FLT_MAX, FLT_MAX });
            ImGui::Begin("Console", &mOpened);

            // Copy stream data to the buffer
            if(stream.str().length() > 0){
                print(stream.str());
                stream.str("");
            }

            // List data
            const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
            if(ImGui::BeginChild("##console_logs", { 0, -footerHeightToReserve }, 0, 0)){
                // Render each text
                static std::vector<std::pair<ImVec4, std::string>> colorStrings;
                static std::vector<bool> newLines;

                ImGui::PushTextWrapPos();
                for(const auto& str : lines){
                    // Initialize with default color
                    colorStrings.push_back({{1, 1, 1, 1}, ""});
                    newLines.push_back(false);

                    // Check for ANSI colors
                    for(size_t i = 0; i < str.length(); i++){
                        const char& c = str[i];

                        if(c == '\033'){
                            // ANSI command follows this char
                            // Next char is [ and the ending char is m. iterate over each char to build the code
                            std::string codeStr = "\033[";
                            int k = i + 2;

                            while(str[k] != 'm' && k < str.length() - 1){
                                codeStr += str[k++];
                            }

                            if(codeStr == "\033[1m"){
                                // Bold isnt supported
                                continue;
                            }

                            codeStr += 'm';
                            colorStrings.push_back({get_ansi_color(codeStr), ""});
                            newLines.push_back(false);
                            i += codeStr.length() - 1;
                            continue;
                        } else if(c == '\n'){
                            // New line char, set new line
                            colorStrings.push_back({colorStrings.back().first, ""});
                            newLines.back() = true;
                            newLines.push_back(false);
                            continue;
                        }

                        colorStrings.back().second += c;
                    }

                    // Render texts
                    for(size_t i = 0; i < colorStrings.size(); i++){
                        auto& [color, text] = colorStrings[i];
                        ImGui::TextColored(color, "%s", text.c_str());
                        
                        // Render on same line if it isnt the end of the split string
                        if(!newLines[i]){
                            ImGui::SameLine(0, 0);
                        }
                    }

                    // Clear vector for next line
                    colorStrings.clear();
                    newLines.clear();
                }
                ImGui::PopTextWrapPos();

                // Auto-scroll logs.
                if(scrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();

            // Input handling
            ImGui::Separator();
            ImGui::SetNextItemWidth(-72);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - 4);

            if(reclaimFocus){
                ImGui::SetKeyboardFocusHere();
                reclaimFocus = false;
            }

            if(ImGui::InputTextWithHint("##", "COMMAND", &inputBuffer[0], 512, ImGuiInputTextFlags_EnterReturnsTrue) && inputBuffer[0] != '\0'){
                run_command();
                reclaimFocus = true;
            }

            ImGui::SameLine();

            if(ImGui::Button("RUN", { 64, ImGui::GetFrameHeight() })){
                run_command();
                reclaimFocus = false;
            }

            ImGui::End();
        }

        // Handle pressing keys
        if(app->keyboard.is_just_pressed(Keyboard::GRAVE))
            mOpened = !mOpened;
    }

    void Console::set_open(bool open){
        mOpened = open;
        reclaimFocus = true;
    }

    void Console::register_cmd(const string& key, ConsoleFunc func){
        commandAliases.push_back(key);
        commandArray.push_back(func);
        Logger::println(Level::INFO, "Console", "Command " + key + " registered");
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
            Logger::println(Level::INFO, "Console", "Command " + key + " unregistered");
        }
    }

    void Console::print(const string& text){
        // Add the text provided to the circular buffer
        lines.push_back(text);

        if(lines.size() > MAX_LINES)
            lines.erase(lines.begin(), lines.begin() + 1);
    }
}