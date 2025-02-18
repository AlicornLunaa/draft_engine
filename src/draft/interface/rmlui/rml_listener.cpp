#include "draft/interface/rmlui/rml_listener.hpp"
#include <cassert>

namespace Draft {
    // Constructor
    RmlListener::RmlListener(const RmlEvent& eventFunc) : m_eventFunc(eventFunc) {
    }
    
    // Functions
    void RmlListener::set_function(const RmlEvent& eventFunc){
        m_eventFunc = eventFunc;
    }
    
    void RmlListener::ProcessEvent(Rml::Event& event){
        if(!m_eventFunc) return;
        m_eventFunc(event);
    }
}