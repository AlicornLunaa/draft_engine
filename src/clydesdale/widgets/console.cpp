#include "console.hpp"
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include <vector>

using namespace sf;
using namespace std;

namespace Clydesdale {
    // Private functions
    void Console::parseArguments(const string& text, vector<string>& args){
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

    string Console::addText(const string& text){
        size_t textLength = 0;
        string out = "";

        for(size_t i = 0; i < text.size(); i++){
            outputBuffer[cursor + i] = text[i];

            if(text[i] == '\0')
                break;

            if(textLength % LINE_WIDTH == 0 && textLength != 0){
                outputBuffer[cursor + i + 1] = '\n';
                cursor++;
            }

            out += text[i];
            textLength++;
        }

        outputBuffer[cursor + textLength] = '\n';
        cursor += textLength + 1;

        if(cursor >= OUTPUT_SIZE)
            cursor = 0;

        return out;
    }

    // Constructors
    Console::Console(bool openByDefault){
        // Open by default flag
        mOpened = openByDefault;

        // Zero out buffers
        for(int i = 0; i < INPUT_SIZE; i++)
            inputBuffer[i] = '\0';
            
        for(int i = 0; i < OUTPUT_SIZE; i++)
            outputBuffer[i] = '\0';
    }

    Console::~Console(){}

    // Functions
    void Console::draw(){
        // Interface
        if(mOpened){
            ImGui::SetNextWindowSizeConstraints({ 300, 200 }, { FLT_MAX, FLT_MAX });
            ImGui::Begin("Console", &mOpened);

            // List data
            ImGui::InputTextMultiline("##console_logs", outputBuffer, OUTPUT_SIZE, ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 2 - 16), ImGuiInputTextFlags_ReadOnly);

            // Input handling
            ImGui::SetNextItemWidth(-72);
            ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - 4);
            ImGui::SetKeyboardFocusHere();
            ImGui::InputTextWithHint("##", "COMMAND", &inputBuffer[0], 512);
            ImGui::SameLine();

            if(ImGui::Button("RUN", { 64, ImGui::GetFrameHeight() }) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && inputBuffer[0] != '\0')){
                string rawCommand(inputBuffer, 512);
                vector<string> argList;
                parseArguments(rawCommand, argList);
                addText(rawCommand);
                run(argList[0], argList);

                for(int i = 0; i < 512; i++)
                    inputBuffer[i] = '\0';
            }

            ImGui::End();
        }

        // Handle pressing keys
        if(Keyboard::isKeyPressed(Keyboard::Tilde) && !mKeyPressed){
            mOpened = !mOpened;
            mKeyPressed = true;

        } else if(!Keyboard::isKeyPressed(Keyboard::Tilde) && mKeyPressed){
            mKeyPressed = false;
        }
    }

    void Console::registerCmd(const string& key, ConsoleFunc func){
        commandAliases.push_back(key);
        commandArray.push_back(func);
    }

    void Console::deleteCmd(const string& key){
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
        }
    }

    void Console::print(const string& text){
        addText(text);
    }
}