#include "draft/ecs/system.hpp"

namespace Draft {
    void SystemRegistry::update_all(Time dt){
        for(auto& type : m_order){
            auto it = m_systems.find(type);
            if(it != m_systems.end())
                it->second->update(dt);
        }
    }

    void SystemRegistry::render_all(Time dt, RenderLayer layer){
        for(auto& type : m_order){
            auto it = m_systems.find(type);
            if(it != m_systems.end() && has_layer(it->second->get_render_layers(), layer))
                it->second->render(dt, layer);
        }
    }

    void SystemRegistry::attach_all(){
        for(auto& type : m_order){
            auto it = m_systems.find(type);
            if(it != m_systems.end())
                it->second->on_attach();
        }
    }

    void SystemRegistry::detach_all(){
        for(auto& type : m_order){
            auto it = m_systems.find(type);
            if(it != m_systems.end())
                it->second->on_detach();
        }
    }

    bool SystemRegistry::dispatch_event(const Event& event){
        for(auto& type : m_order){
            auto it = m_systems.find(type);
            if(it != m_systems.end() && it->second->on_event(event))
                return true;
        }

        return false;
    }
}
