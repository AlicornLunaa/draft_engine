#include "draft/core/command_catalog.hpp"

namespace Draft {
    void CommandCatalog::register_command(CommandInfo info){
        auto it = m_byName.find(info.name);

        if(it == m_byName.end()){
            m_byName[info.name] = m_order.size();
            m_order.push_back(std::move(info));
        } else {
            m_order[it->second] = std::move(info);
        }
    }

    const CommandInfo* CommandCatalog::find(std::string_view name) const {
        auto it = m_byName.find(std::string(name));
        return it == m_byName.end() ? nullptr : &m_order[it->second];
    }
}
