#pragma once

#include "RmlUi/Core/DataModelHandle.h"
#include "draft/interface/rmlui/rml_system.hpp"
#include "draft/input/event.hpp"
#include "draft/math/glm.hpp"

#include "RmlUi/Core/ElementDocument.h"
#include "RmlUi/Core/Context.h"
#include "draft/util/files/file_handle.hpp"
#include "draft/util/reflectable.hpp"

#include <string>
#include <type_traits>
#include <vector>

namespace Draft {
    namespace detail {
        template<typename T>
        struct is_vector : std::false_type {};

        template<typename T, typename Alloc>
        struct is_vector<std::vector<T, Alloc>> : std::true_type {};

        template<typename T>
        constexpr bool is_vector_v = is_vector<T>::value;
    }

    /**
     * @brief One Rml::Context, independently constructible so more than one can coexist (e.g.
     * one per viewport).
     */
    class RmlContext {
    private:
        // Variables
        Rml::Context* m_context = nullptr;

        // Private functions
        static int get_modifiers(int draft_modifiers);
        static Rml::Input::KeyIdentifier get_key(int draft_key);

        // Registers T with the constructor's type register if it hasn't been already, recursing
        // into vector element types and reflectable members so nested data can be bound too.
        template<typename T>
        static void register_reflected_type(Rml::DataModelConstructor& constructor){
            if constexpr(detail::is_vector_v<T>){
                register_reflected_type<typename T::value_type>(constructor);

                if(!constructor.GetDataTypeRegister()->GetDefinition<T>())
                    constructor.RegisterArray<T>();
            } else if constexpr(Reflectable<T>){
                if(constructor.GetDataTypeRegister()->GetDefinition<T>())
                    return;

                auto handle = constructor.RegisterStruct<T>();
                std::apply([&](auto&&... field){
                    (register_reflected_field(constructor, handle, field), ...);
                }, T::reflect());
            }
        }

        template<typename Object, typename MemberType>
        static void register_reflected_field(Rml::DataModelConstructor& constructor, Rml::StructHandle<Object>& handle, const Field<Object, MemberType>& field){
            register_reflected_type<MemberType>(constructor);
            handle.RegisterMember(std::string(field.name), field.pointer);
        }

        template<typename T> requires Reflectable<T>
        void recursive_data_model(Rml::DataModelConstructor& constructor, T& data){
            for_each_field(data, [&](std::string_view name, auto& field){
                register_reflected_type<std::remove_cvref_t<decltype(field)>>(constructor);
                constructor.Bind(std::string(name), &field);
            });
        }

    public:
        // Constructors
        RmlContext(const std::string& name, const Vector2i& size);
        RmlContext(const RmlContext& other) = delete;
        RmlContext(RmlContext&& other);
        virtual ~RmlContext();

        // Operators
        RmlContext& operator=(const RmlContext& other) = delete;
        RmlContext& operator=(RmlContext&& other);

        // Functions
        template<typename T> requires Reflectable<T>
        Rml::DataModelHandle create_data_model(const std::string& name, T& data){
            auto constructor = create_data_model(name);
            recursive_data_model(constructor, data);
            return constructor.GetModelHandle();
        }

        Rml::DataModelConstructor create_data_model(const std::string& name) const;
        Rml::ElementDocument* create_document(const std::string& name) const;
        Rml::ElementDocument* load_document(const FileHandle& path) const;
        virtual bool handle_event(const Event& event); // Returns true if this context used an event, false if it should continue to others
        void render() const;
        bool is_valid() const;
        bool wants_keyboard_capture() const; // True if a text-editable element (input/textarea/select) has focus
        bool wants_mouse_capture() const; // True if the mouse is hovering an element in this context
        inline Rml::Context* get_context() const { return m_context; }
    };
}
