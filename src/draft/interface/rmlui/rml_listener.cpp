#include "draft/interface/rmlui/rml_listener.hpp"
#include <cassert>

namespace Draft {
    // Constructor
    RmlListener::RmlListener(RmlEvent& eventFunc) : eventFunc(eventFunc) {
    }
    
    // Functions
    void RmlListener::ProcessEvent(Rml::Event& event){
        if(!eventFunc) return;
        eventFunc(event);
    }
}