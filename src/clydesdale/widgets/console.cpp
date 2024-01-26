#include "console.hpp"
#include <clydesdale/util/constants.hpp>
#include <SFML/Graphics.hpp>
#include <imgui.h>

using namespace sf;
using namespace std;
using namespace Clydesdale::Widgets;

// Static variables
char Console::inputBuffer[512];

// Private functions
void Console::addText(const string& txt){
    logQueue.push_back(txt);

    if(logQueue.size() > CONSOLE_MAX_LINES){
        logQueue.erase(logQueue.begin(), logQueue.begin() + 1);
    }
}

// Constructors
Console::Console(){}
Console::~Console(){}

// Functions
void Console::draw(){
    // Interface
    ImGui::Begin("Console");

    // List data
    for(const string& line : logQueue)
        ImGui::TextWrapped(line.c_str());

    // Input handling
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##", "COMMAND", &inputBuffer[0], 512);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if(ImGui::Button("RUN") || sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)){
        addText(string(inputBuffer, 512));
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
    if(!commands.contains(key))
        return false;

    addText("Ran command: " + key);
    commands[key]();

    return true;
}