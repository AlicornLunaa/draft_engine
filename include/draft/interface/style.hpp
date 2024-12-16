#pragma once

#include "draft/math/glm.hpp"
#include "draft/rendering/font.hpp"

#include <string>
#include <unordered_map>

namespace Draft {
    namespace UI {
        enum Anchor { TOP, MIDDLE, BOTTOM, LEFT, CENTER, RIGHT };

        template<typename T>
        struct Property {
            // Used to define a simple class with properties that be enabled or disabled
            // without needing pointers or anything else like that
            T value;
            bool enabled = false;

            Property<T>& operator=(const T& value){
                this->value = value;
                enabled = true;
                return *this;
            }

            Property<T>& operator=(const Property<T>& other){
                // This is used for cascading with the style struct. If the other is enabled, this should be set to
                // other, but if its not enabled, keep what we had.
                if(other.enabled){
                    value = other.value;
                    enabled = true;
                }

                return *this;
            }
        };
        
        struct Style {
            // Common stylesheet for a ui interface
            Property<Anchor> horizontalAnchor;
            Property<Anchor> verticalAnchor;
            
            Property<Vector4f> textColor;
            Property<Font*> font;

            Property<Vector4f> activeColor;
            Property<Vector4f> inactiveColor;
            Property<Vector4f> disabledColor;

            Property<Vector2f> padding;
            Property<Vector4f> margin;
            
            Property<Vector4f> backgroundColor;
            Property<Texture const*> background;
        };

        class Stylesheet {
        private:
            std::unordered_map<std::string, Style> m_classMap;

            const std::vector<std::string> explode(const std::string& str, const char c = ' ') const;

        public:
            // Constructs default style, which is different than *
            Stylesheet();

            /**
             * @brief Adds a single class for a simple css style system
             * @param name Either * or some name which is indexed by widgets
             * @param style Rendering guidelines
             */
            void add_style(const std::string& name, const Style& style);

            /**
             * @brief Removes the class provided from the stylesheet
             * @param name 
             */
            void remove_style(const std::string& name);

            /**
             * @brief Accepts a list of classes and creates a cascaded style for it
             * @param identifiers 
             * @return Style 
             */
            Style get_style(const std::string& identifiers) const;
        };
    };
};