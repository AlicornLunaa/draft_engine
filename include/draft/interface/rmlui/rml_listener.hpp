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
        RmlEvent m_eventFunc = nullptr;

    public:
        // Constructors
        RmlListener() = default;
        RmlListener(const RmlEvent& eventFunc);

        // Functions
        void set_function(const RmlEvent& eventFunc);
	    void ProcessEvent(Rml::Event& event);
    };
};