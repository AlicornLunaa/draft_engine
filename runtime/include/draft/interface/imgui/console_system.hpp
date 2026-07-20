#pragma once

#include "draft/ecs/system.hpp"
#include "draft/util/circular_buffer.hpp"
#include "draft/util/logger.hpp"
#include "draft/util/reflectable.hpp"

#include <array>
#include <cstddef>
#include <string>

struct ImGuiInputTextCallbackData;

namespace Draft {
    class Engine;
    class ApplicationInterface;
    class AssetManager;

    /**
     * @brief Log/command console, hidden by default and toggled with the grave accent key.
     * Logger output is captured through a sink into a bounded ring buffer; the input box runs
     * whatever's typed through @p engine's CommandCatalog.
     */
    class ConsoleSystem : public AbstractSystem {
    public:
        ConsoleSystem(Engine& engine, ApplicationInterface& application, AssetManager& assets);
        ~ConsoleSystem() override;

        void render(Time dt, RenderLayer layer) override;

        DRAFT_REFLECTABLE(ConsoleSystem)

    private:
        struct LogEntry {
            LogLevel level;
            std::string name;
            std::string message;
        };

        void register_builtin_commands();
        void draw_log();
        void draw_input();
        void run_command_line(const std::string& line);
        int handle_text_edit(ImGuiInputTextCallbackData* data);
        static int text_edit_callback(ImGuiInputTextCallbackData* data);

        Engine& m_engine;
        ApplicationInterface& m_application;
        AssetManager& m_assets;
        CircularBuffer<LogEntry> m_entries;
        std::size_t m_sinkToken;
        std::array<char, 256> m_inputBuffer{};
        bool m_visible = false;
        bool m_autoScroll = true;
        bool m_scrollToBottom = false;
    };
}
