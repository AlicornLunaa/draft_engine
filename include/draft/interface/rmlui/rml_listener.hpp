#pragma once

#include "RmlUi/Core/EventListener.h"
#include <functional>
namespace Draft {
    /// Type for function pointer of listening function
    typedef std::function<void(Rml::Event& event)> RmlEvent;

    /// Simple listener to redirect to std::function, slower but more convenient
    class RmlListener : public Rml::EventListener {
    private:
        // Variables
        RmlEvent eventFunc;

    public:
        // Constructors
        RmlListener(RmlEvent& eventFunc);

        // Functions
	    void ProcessEvent(Rml::Event& event);
    };
};