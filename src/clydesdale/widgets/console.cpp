#include "console.hpp"
#include <clydesdale/util/constants.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>

using namespace sf;
using namespace std;
using namespace Clydesdale::Widgets;

// Private functions
string Console::addText(const string& txt){
    size_t textLength = 0;
    string out = "";

    for(size_t i = 0; i < txt.size(); i++){
        outputBuffer[cursor + i] = txt[i];
        out += txt[i];

        if(txt[i] == '\0')
            break;

        if(textLength % LINE_WIDTH == 0 && textLength != 0){
            outputBuffer[cursor + i + 1] = '\n';
            cursor++;
        }

        textLength++;
    }

    outputBuffer[cursor + textLength] = '\n';
    cursor += textLength + 1;

    if(cursor >= OUTPUT_SIZE)
        cursor = 0;

    return out;
}

// Constructors
Console::Console(){
    // Zero out buffers
    for(int i = 0; i < INPUT_SIZE; i++)
        inputBuffer[i] = '\0';
        
    for(int i = 0; i < OUTPUT_SIZE; i++)
        outputBuffer[i] = '\0';

    addText("/*\n"
            " The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
            " the hexadecimal encoding of one offending instruction,\n"
            " more formally, the invalid operand with locked CMPXCHG8B\n"
            " instruction bug, is a design flaw in the majority of\n"
            " Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
            " processors (all in the P5 microarchitecture).\n"
            "*/\n\n"
            "label:\n"
            "\tlock cmpxchg8b eax\n");
}

Console::~Console(){}

// Functions
void Console::draw(){
    // Interface
    ImGui::Begin("Console");

    // List data
    ImGui::InputTextMultiline("##console_logs", outputBuffer, OUTPUT_SIZE, ImVec2(-FLT_MIN, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 2 - 16), ImGuiInputTextFlags_ReadOnly);

    // Input handling
    ImGui::SetNextItemWidth(-72);
    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() - 4);
    ImGui::InputTextWithHint("##", "COMMAND", &inputBuffer[0], 512);
    ImGui::SameLine();
    if(ImGui::Button("RUN", { 64, ImGui::GetFrameHeight() }) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && inputBuffer[0] != '\0')){
        run(addText(string(inputBuffer, 512)));

        for(int i = 0; i < 512; i++)
            inputBuffer[i] = '\0';
    }

    ImGui::End();
}

void Console::registerCmd(std::string key, std::function<void(void)> func){
    commands[key] = func;
}

void Console::deleteCmd(std::string key){
    commands.erase(key);
}

void Console::print(std::string txt){
    addText(txt);
}

bool Console::run(std::string key){
    // TODO: this is so fucked please fix it
    if(!commands.count(key) == 0)
        return false;

    addText("Ran command: " + key);
    commands[key]();

    return true;
}