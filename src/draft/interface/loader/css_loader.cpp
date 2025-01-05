#include "draft/interface/loader/css_loader.hpp"
#include "draft/interface/loader/css_style.hpp"
#include "draft/interface/unit_value.hpp"
#include "draft/util/file_handle.hpp"
#include "draft/util/color.hpp"

#include <utility>
#include <string>
#include <vector>

namespace Draft::UI {
    namespace CSSLoader {
        /**
        * Parses CSS with the anatomy, button=selector, hover=pseudo-class, background-color=property, #FFF=value
        * button:hover {
        *     background-color: #FFF;
        * }
        */
        // Types
        enum FiniteState { START, SELECTORS, PSEUDO, PROPERTY, VALUE, END };

        typedef std::vector<std::pair<std::string, FiniteState>> TokenList;

        // Private functions
        const std::vector<std::string> explode(const std::string& str, const char c){
            std::string buf{""};
            std::vector<std::string> v;
            
            for(char n : str){
                if(n != c){
                    buf += n;
                } else if(buf != "") {
                    v.push_back(buf);
                    buf = "";
                }
            }

            if(buf != "")
                v.push_back(buf);
            
            return v;
        }

        void state_machine(FiniteState& state, char ch){
            switch(state){
                case START:
                    if(ch != ' ') state = SELECTORS;
                    break;

                case SELECTORS:
                    if(ch == '{') state = PROPERTY;
                    if(ch == ':') state = PSEUDO;
                    break;

                case PSEUDO:
                    if(ch == '{') state = PROPERTY;
                    break;

                case PROPERTY:
                    if(ch == ':') state = VALUE;
                    if(ch == '}') state = END;
                    break;

                case VALUE:
                    if(ch == ';') state = PROPERTY;
                    break;

                case END:
                    break;
            }
        }

        TokenList tokenize(std::string data){
            FiniteState lastState = START;
            FiniteState state = START;
            std::string buf = "";
            TokenList list;

            for(auto& ch : data){
                bool garbageChar = ch == '\n' || ch == '{' || ch == '}' || ch == ':' || ch == ';';
                state_machine(state, ch);

                if(state == END){
                    state = SELECTORS;
                }

                if(lastState != state){
                    if(!buf.empty()) list.push_back({buf, lastState});
                    lastState = state;
                    buf = "";
                }

                if(state == SELECTORS && ch == ' '){
                    // For the descendant selector each class is delimited by a space
                    if(!buf.empty()) list.push_back({buf, state});
                    buf = "";
                }

                if(state != START && state != END && !garbageChar && ch != ' '){
                    buf += ch;
                }
            }

            return list;
        }

        PseudoSelector parse_pseudo(const std::string& pseudo){
            if(pseudo == "hover"){
                return HOVER;
            }

            return NONE;
        }

        std::any parse_value(const std::string& property, const std::string& value){
            // Property-specifics
            if(property == "box-sizing"){
                if(value == "content-box") return CSSBoxSizing::CONTENT_BOX;
                if(value == "border-box") return CSSBoxSizing::BORDER_BOX;
            }

            // True/false parsing
            if(value == "true"){
                return true;
            } else if(value == "false"){
                return false;
            }

            // Generics
            if(value[0] == '#'){
                return Color(value);
            } else if(value.starts_with("rgb(") && value.ends_with(")")){
                auto v = explode(value.substr(4, value.size() - 5), ',');
                return Color(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]), 1);
            } else if(value.starts_with("rgba(") && value.ends_with(")")){
                auto v = explode(value.substr(5, value.size() - 6), ',');
                return Color(std::stof(v[0]), std::stof(v[1]), std::stof(v[2]), std::stof(v[3]));
            } else if(value.starts_with("url(") && value.ends_with(")")){
                return value.substr(4, value.size() - 5);
            } else if(value.ends_with("px")){
                return UnitValue{std::stof(value), 0.f};
            } else if(value.ends_with("%")){
                return UnitValue{0.f, std::stof(value)};
            }

            return value;
        }

        // Public interface
        void parse_file(Stylesheet& style, const FileHandle& handle){
            // Performs a lexical analysis on the file for css then builds a stylesheet object out of it
            TokenList list = tokenize(handle.read_string());

            std::vector<std::string> selectors;
            PseudoSelector pseudo = NONE;
            std::string property;
            bool resetOnNextSelector = false;

            for(auto& [data, state] : list){
                if(state == SELECTORS){
                    if(resetOnNextSelector){
                        selectors.clear();
                        pseudo = PseudoSelector::NONE;
                        resetOnNextSelector = false;
                    }

                    selectors.push_back(data);
                } else if(state == PSEUDO){
                    pseudo = parse_pseudo(data);
                } else if(state == PROPERTY){
                    property = data;
                } else if(state == VALUE){
                    // Shorthand overrides for 4x values
                    if(property == "padding" || property == "margin"){
                        auto v = explode(data, ' ');

                        if(v.size() == 4){
                            // Set all four
                            style.set(selectors, pseudo, property + "-left", parse_value(property, v[0]));
                            style.set(selectors, pseudo, property + "-top", parse_value(property, v[1]));
                            style.set(selectors, pseudo, property + "-right", parse_value(property, v[2]));
                            style.set(selectors, pseudo, property + "-bottom", parse_value(property, v[3]));
                        } else if(v.size() == 2){
                            // Set top/bottom then left/right
                            style.set(selectors, pseudo, property + "-left", parse_value(property, v[1]));
                            style.set(selectors, pseudo, property + "-right", parse_value(property, v[1]));
                            style.set(selectors, pseudo, property + "-top", parse_value(property, v[0]));
                            style.set(selectors, pseudo, property + "-bottom", parse_value(property, v[0]));
                        } else {
                            // Set all
                            style.set(selectors, pseudo, property + "-left", parse_value(property, v[0]));
                            style.set(selectors, pseudo, property + "-top", parse_value(property, v[0]));
                            style.set(selectors, pseudo, property + "-right", parse_value(property, v[0]));
                            style.set(selectors, pseudo, property + "-bottom", parse_value(property, v[0]));
                        }

                        continue;
                    }

                    style.set(selectors, pseudo, property, parse_value(property, data));
                    resetOnNextSelector = true;
                }
            }
        }
    };
};