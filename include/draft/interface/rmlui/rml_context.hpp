#pragma once

#include "RmlUi/Core/ElementDocument.h"
#include "draft/input/event.hpp"
#include "draft/interface/rmlui/rml_engine.hpp"

#include "RmlUi/Core/Context.h"
#include "draft/math/glm.hpp"
#include <string>

namespace Draft {
    class RmlContext {
    private:
        // Variables
        RmlEngine* m_engine = nullptr;
        Rml::Context* m_context = nullptr;

        // Private functions
        static int get_modifiers(int draft_modifiers);
        static Rml::Input::KeyIdentifier get_key(int draft_key);

    public:
        // Constructors
        RmlContext(RmlEngine& engine, const std::string& name, const Vector2i& size);
        RmlContext(const RmlContext& other) = delete;
        RmlContext(RmlContext&& other);
        ~RmlContext();

        // Operators
        RmlContext& operator=(const RmlContext& other) = delete;
        RmlContext& operator=(RmlContext&& other);

        // Functions
        Rml::ElementDocument* create_document(const std::string& name) const;
        Rml::ElementDocument* load_document(const std::string& path) const;
        void handle_event(const Event& event);
        void render() const;
        bool is_valid() const;
    };
}