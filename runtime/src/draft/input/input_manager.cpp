#include "draft/input/input_manager.hpp"

namespace Draft {
    namespace {
        const std::vector<InputBinding> EMPTY_BINDINGS;

        std::string source_to_string(InputSource source){
            switch(source){
                case InputSource::Keyboard: return "keyboard";
                case InputSource::Mouse: return "mouse";
            }

            return "keyboard";
        }

        InputSource source_from_string(const std::string& str){
            return str == "mouse" ? InputSource::Mouse : InputSource::Keyboard;
        }
    }

    InputManager::InputManager(Keyboard& keyboardRef, Mouse& mouseRef) : keyboardRef(keyboardRef), mouseRef(mouseRef) {}

    bool InputManager::is_binding_pressed(const InputBinding& binding) const {
        switch(binding.source){
            case InputSource::Keyboard: return keyboardRef.is_pressed(binding.code);
            case InputSource::Mouse: return mouseRef.is_pressed(binding.code);
        }

        return false;
    }

    bool InputManager::is_binding_just_pressed(const InputBinding& binding) const {
        switch(binding.source){
            case InputSource::Keyboard: return keyboardRef.is_just_pressed(binding.code);
            case InputSource::Mouse: return mouseRef.is_just_pressed(binding.code);
        }

        return false;
    }

    void InputManager::bind_default(const std::string& action, InputBinding binding){
        bind_default(action, {binding});
    }

    void InputManager::bind_default(const std::string& action, std::initializer_list<InputBinding> bindings){
        std::vector<InputBinding> list(bindings);
        m_defaultBindings[action] = list;

        // Only seed the current binding on first registration, as to not affect already bound
        if(!m_bindings.contains(action))
            m_bindings[action] = std::move(list);
    }

    void InputManager::set_bindings(const std::string& action, std::vector<InputBinding> bindings){
        m_bindings[action] = std::move(bindings);
    }

    void InputManager::add_binding(const std::string& action, InputBinding binding){
        m_bindings[action].push_back(binding);
    }

    void InputManager::remove_binding(const std::string& action, const InputBinding& binding){
        auto it = m_bindings.find(action);
        if(it == m_bindings.end())
            return;

        std::erase(it->second, binding);
    }

    void InputManager::clear_bindings(const std::string& action){
        m_bindings[action].clear();
    }

    void InputManager::reset_to_defaults(){
        for(auto& [action, bindings] : m_defaultBindings)
            m_bindings[action] = bindings;
    }

    void InputManager::reset_to_defaults(const std::string& action){
        auto it = m_defaultBindings.find(action);
        if(it != m_defaultBindings.end())
            m_bindings[action] = it->second;
    }

    bool InputManager::has_action(const std::string& action) const {
        return m_bindings.contains(action);
    }

    const std::vector<InputBinding>& InputManager::get_bindings(const std::string& action) const {
        auto it = m_bindings.find(action);
        return it == m_bindings.end() ? EMPTY_BINDINGS : it->second;
    }

    const std::vector<InputBinding>& InputManager::get_default_bindings(const std::string& action) const {
        auto it = m_defaultBindings.find(action);
        return it == m_defaultBindings.end() ? EMPTY_BINDINGS : it->second;
    }

    bool InputManager::is_pressed(const std::string& action) const {
        auto it = m_bindings.find(action);
        if(it == m_bindings.end())
            return false;

        for(const InputBinding& binding : it->second){
            if(is_binding_pressed(binding))
                return true;
        }

        return false;
    }

    bool InputManager::is_just_pressed(const std::string& action) const {
        auto it = m_bindings.find(action);
        if(it == m_bindings.end())
            return false;

        bool result = false;
        for(const InputBinding& binding : it->second)
            result |= is_binding_just_pressed(binding);

        return result;
    }

    JSON InputManager::save_bindings() const {
        JSON json = JSON::object();

        for(auto& [action, bindings] : m_bindings){
            JSON array = JSON::array();

            for(auto& binding : bindings){
                array.push_back(JSON::object({
                    {"source", source_to_string(binding.source)},
                    {"code", binding.code}
                }));
            }

            json[action] = std::move(array);
        }

        return json;
    }

    void InputManager::save_bindings(const FileHandle& file) const {
        file.write_string(save_bindings().dump(4));
    }

    void InputManager::load_overrides(const JSON& json){
        for(auto& [action, array] : json.items()){
            std::vector<InputBinding> bindings;
            bindings.reserve(array.size());

            for(auto& entry : array){
                InputBinding binding;
                binding.source = source_from_string(entry.at("source").get<std::string>());
                binding.code = entry.at("code").get<int>();
                bindings.push_back(binding);
            }

            m_bindings[action] = std::move(bindings);
        }
    }

    void InputManager::load_overrides(const FileHandle& file){
        load_overrides(JSON(file));
    }
}
