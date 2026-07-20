#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Draft {
    class Engine;
    class ApplicationInterface;
    class AssetManager;

    /**
     * @brief Everything a command handler runs with, mirroring GameContext (minus its
     * PersistentStore) since that's already what a game module's own registration entry point
     * gets, plus its own arguments (the command line split on whitespace, name excluded).
     */
    struct CommandContext {
        Engine& engine;
        ApplicationInterface& application;
        AssetManager& assets;
        std::vector<std::string> args;
    };

    using CommandHandler = std::function<void(CommandContext&)>;

    struct CommandInfo {
        std::string name;
        std::string description;
        CommandHandler handler;
    };

    /**
     * @brief Flat registry of named, callable commands, alongside ComponentCatalog/SystemCatalog
     * on Engine. Unlike those, a command has no live instance to construct or serialize, it's
     * just a name and a function.
     */
    class CommandCatalog {
    public:
        /**
         * @brief Registers @p info under its own name, replacing any existing command already
         * registered under that name.
         */
        void register_command(CommandInfo info);

        /**
         * @brief Gets the command registered under @p name, or nullptr if none is.
         */
        const CommandInfo* find(std::string_view name) const;

        /**
         * @brief Every registered command, in registration order.
         */
        const std::vector<CommandInfo>& all() const { return m_order; }

    private:
        std::vector<CommandInfo> m_order;
        std::unordered_map<std::string, std::size_t> m_byName;
    };
}
