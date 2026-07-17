#pragma once

#include "draft/ecs/system.hpp"
#include "draft/input/keyboard.hpp"
#include "draft/input/mouse.hpp"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/json.hpp"
#include "draft/util/reflectable.hpp"

#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>

namespace Draft {
    /**
     * @brief Which physical device an InputBinding's code is interpreted against.
     */
    enum class InputSource {
        Keyboard,
        Mouse
    };

    /**
     * @brief One physical key/button an action can be triggered by
     */
    struct InputBinding {
        InputSource source = InputSource::Keyboard;
        int code = 0;

        bool operator==(const InputBinding& other) const {
            return source == other.source && code == other.code;
        }
    };

    /**
     * @brief Maps named actions to one or more rebindable InputBindings, so gameplay code queries
     * an action by name (e.g. "Jump") instead of a hardcoded Keyboard::Key/Mouse::Button.
     *
     * Game code declares its defaults once via bind_default(), normally right after construction.
     * From then on, is_pressed()/is_just_pressed() go through the action's current bindings,
     * which start out equal to the defaults and can be changed at runtime (add_binding(),
     * set_bindings(), reset_to_defaults()) without touching gameplay code. save_bindings()/
     * load_overrides() persist and restore that current state as JSON, independent of whatever
     * bind_default() calls run on the next launch.
     */
    class InputManager : public AbstractSystem {
    private:
        Keyboard& keyboardRef;
        Mouse& mouseRef;

        std::unordered_map<std::string, std::vector<InputBinding>> m_defaultBindings;
        std::unordered_map<std::string, std::vector<InputBinding>> m_bindings;

        bool is_binding_pressed(const InputBinding& binding) const;
        bool is_binding_just_pressed(const InputBinding& binding) const;

    public:
        InputManager(Keyboard& keyboardRef, Mouse& mouseRef);

        /**
         * @brief Declares @p action's default binding(s), meant to be called from game code once
         * at startup for every action it defines. Also becomes @p action's current binding the
         * first time @p action is seen, so is_pressed() works immediately, even before
         * load_overrides() runs.
         */
        void bind_default(const std::string& action, InputBinding binding);
        void bind_default(const std::string& action, std::initializer_list<InputBinding> bindings);

        /**
         * @brief Replaces @p action's current binding(s) wholesale, e.g. from a rebinding UI that
         * just captured a new key. Does not touch the default(s) set by bind_default().
         */
        void set_bindings(const std::string& action, std::vector<InputBinding> bindings);

        /**
         * @brief Adds one more binding to @p action's current set, on top of whatever it already has.
         */
        void add_binding(const std::string& action, InputBinding binding);

        /**
         * @brief Removes a single matching binding from @p action's current set, if present.
         */
        void remove_binding(const std::string& action, const InputBinding& binding);

        /**
         * @brief Clears every current binding for @p action, without touching its defaults.
         */
        void clear_bindings(const std::string& action);

        /**
         * @brief Restores every action's current bindings back to what bind_default() declared.
         */
        void reset_to_defaults();

        /**
         * @brief Restores @p action's current bindings back to what bind_default() declared for it.
         */
        void reset_to_defaults(const std::string& action);

        bool has_action(const std::string& action) const;
        const std::vector<InputBinding>& get_bindings(const std::string& action) const;
        const std::vector<InputBinding>& get_default_bindings(const std::string& action) const;

        /**
         * @brief True if any of @p action's current bindings is currently held down.
         */
        bool is_pressed(const std::string& action) const;

        /**
         * @brief True if any of @p action's current bindings transitioned to pressed this frame.
         */
        bool is_just_pressed(const std::string& action) const;

        /**
         * @brief Dumps every action's current bindings (the "current setup") to JSON, keyed by
         * action name.
         */
        JSON save_bindings() const;
        void save_bindings(const FileHandle& file) const;

        /**
         * @brief Applies @p json on top of the current bindings: every action present in @p json
         * has its current binding(s) replaced. Actions bind_default() declared but @p json
         * doesn't mention are left as they were.
         */
        void load_overrides(const JSON& json);
        void load_overrides(const FileHandle& file);

        DRAFT_REFLECTABLE(InputManager)
    };
}
