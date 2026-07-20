#include "draft/interface/imgui/console_system.hpp"
#include "draft/core/command_catalog.hpp"
#include "draft/core/engine.hpp"

#include "imgui.h"

#include <exception>
#include <string_view>
#include <vector>

namespace Draft {
    namespace {
        constexpr std::size_t LOG_CAPACITY = 512;

        ImVec4 level_color(LogLevel level){
            switch(level){
                case LogLevel::Warning: return ImVec4(0.9f, 0.8f, 0.2f, 1.f);
                case LogLevel::Severe: return ImVec4(0.9f, 0.3f, 0.3f, 1.f);
                case LogLevel::Critical: return ImVec4(1.f, 0.4f, 0.9f, 1.f);
                default: return ImVec4(0.8f, 0.8f, 0.8f, 1.f);
            }
        }

        std::vector<std::string> split_args(const std::string& line){
            std::vector<std::string> tokens;
            std::size_t i = 0;

            while(i < line.size()){
                while(i < line.size() && line[i] == ' ') i++;
                std::size_t start = i;
                while(i < line.size() && line[i] != ' ') i++;

                if(i > start)
                    tokens.push_back(line.substr(start, i - start));
            }

            return tokens;
        }
    }

    ConsoleSystem::ConsoleSystem(Engine& engine, ApplicationInterface& application, AssetManager& assets)
        : m_engine(engine), m_application(application), m_assets(assets), m_entries(LOG_CAPACITY)
    {
        m_sinkToken = Logger::add_sink([this](LogLevel level, std::string_view name, std::string_view message){
            std::string trimmed(message);
            if(!trimmed.empty() && trimmed.back() == '\n')
                trimmed.pop_back();

            std::lock_guard lock(m_entriesMutex);
            m_entries.push(LogEntry{level, std::string(name), std::move(trimmed)});
            m_scrollToBottom = true;
        });

        register_builtin_commands();
    }

    ConsoleSystem::~ConsoleSystem(){
        Logger::remove_sink(m_sinkToken);
    }

    void ConsoleSystem::register_builtin_commands(){
        m_engine.register_command({
            "clear", "Clears the console log.",
            [this](CommandContext&){
                std::lock_guard lock(m_entriesMutex);
                m_entries = CircularBuffer<LogEntry>(LOG_CAPACITY);
            }
        });

        m_engine.register_command({
            "help", "Lists every available command.",
            [](CommandContext& ctx){
                for(const CommandInfo& info : ctx.engine.commands().all())
                    Logger::println(LogLevel::Info, "Console", info.name + " - " + info.description);
            }
        });
    }

    void ConsoleSystem::render(Time dt, RenderLayer layer){
        if(layer != RenderLayer::Default)
            return;

        if(ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false))
            m_visible = !m_visible;

        if(!m_visible)
            return;

        ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

        if(ImGui::Begin("Console", &m_visible)){
            if(ImGui::SmallButton("Clear")){
                std::lock_guard lock(m_entriesMutex);
                m_entries = CircularBuffer<LogEntry>(LOG_CAPACITY);
            }

            ImGui::SameLine();
            ImGui::Checkbox("Auto scroll", &m_autoScroll);

            ImGui::Separator();

            draw_log();
            draw_input();
        }

        ImGui::End();
    }

    void ConsoleSystem::draw_log(){
        ImGui::BeginChild("##ConsoleLog", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders);

        {
            std::lock_guard lock(m_entriesMutex);
            for(std::size_t i = 0; i < m_entries.length(); i++){
                const LogEntry& entry = m_entries[static_cast<int>(i)];
                ImGui::PushStyleColor(ImGuiCol_Text, level_color(entry.level));
                ImGui::TextUnformatted(("[" + entry.name + "] " + entry.message).c_str());
                ImGui::PopStyleColor();
            }
        }

        if(m_scrollToBottom || (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.f);
        m_scrollToBottom = false;

        ImGui::EndChild();
    }

    void ConsoleSystem::draw_input(){
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion;
        ImGui::SetNextItemWidth(-1.f);

        bool submitted = ImGui::InputText("##ConsoleInput", m_inputBuffer.data(), m_inputBuffer.size(), flags, &ConsoleSystem::text_edit_callback, this);

        if(submitted){
            std::string line(m_inputBuffer.data());
            m_inputBuffer[0] = '\0';
            run_command_line(line);
            ImGui::SetKeyboardFocusHere(-1);
        }
    }

    void ConsoleSystem::run_command_line(const std::string& line){
        std::vector<std::string> tokens = split_args(line);
        if(tokens.empty())
            return;

        Logger::println(LogLevel::Info, "Console", "> " + line);

        const CommandInfo* command = m_engine.commands().find(tokens.front());
        if(!command){
            Logger::println(LogLevel::Severe, "Console", "Unknown command: " + tokens.front());
            return;
        }

        CommandContext ctx{m_engine, m_application, m_assets, std::vector<std::string>(tokens.begin() + 1, tokens.end())};

        try {
            command->handler(ctx);
        } catch(const std::exception& e){
            Logger::println(LogLevel::Severe, "Console", std::string(command->name) + ": " + e.what());
        }
    }

    int ConsoleSystem::text_edit_callback(ImGuiInputTextCallbackData* data){
        return static_cast<ConsoleSystem*>(data->UserData)->handle_text_edit(data);
    }

    int ConsoleSystem::handle_text_edit(ImGuiInputTextCallbackData* data){
        if(data->EventFlag != ImGuiInputTextFlags_CallbackCompletion)
            return 0;

        // Only complete the command name itself (the first word), not arguments after it.
        std::string_view typed(data->Buf, static_cast<std::size_t>(data->CursorPos));
        if(typed.find(' ') != std::string_view::npos)
            return 0;

        std::vector<std::string_view> matches;
        for(const CommandInfo& info : m_engine.commands().all())
            if(info.name.starts_with(typed))
                matches.push_back(info.name);

        if(matches.empty())
            return 0;

        if(matches.size() == 1){
            data->DeleteChars(0, data->CursorPos);
            data->InsertChars(0, matches[0].data(), matches[0].data() + matches[0].size());
            data->InsertChars(data->CursorPos, " ");
            return 0;
        }

        std::string_view prefix = matches[0];
        for(std::string_view candidate : matches){
            std::size_t i = 0;
            while(i < prefix.size() && i < candidate.size() && prefix[i] == candidate[i]) i++;
            prefix = prefix.substr(0, i);
        }

        if(prefix.size() > typed.size()){
            data->DeleteChars(0, data->CursorPos);
            data->InsertChars(0, prefix.data(), prefix.data() + prefix.size());
        }

        Logger::println(LogLevel::Info, "Console", "Matches:");
        for(std::string_view candidate : matches)
            Logger::println(LogLevel::Info, "Console", "  " + std::string(candidate));

        return 0;
    }
}
