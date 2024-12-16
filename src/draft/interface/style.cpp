#include "draft/interface/style.hpp"
#include <cassert>
#include <string>
#include <vector>

namespace Draft::UI {
    const std::vector<std::string> Stylesheet::explode(const std::string& str, const char c) const {
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

    Stylesheet::Stylesheet(){
        Style defaultStyle;
        defaultStyle.horizontalAnchor = LEFT;
        defaultStyle.verticalAnchor = TOP;
        
        defaultStyle.textColor = {1, 1, 1, 1};
        defaultStyle.font = nullptr;

        defaultStyle.activeColor = {0.4, 0.8, 0.4, 1};
        defaultStyle.inactiveColor = {0.8, 0.4, 0.4, 1};
        defaultStyle.disabledColor = {0.4, 0.4, 0.4, 1};

        defaultStyle.padding = {0, 0};
        defaultStyle.margin = {0, 0, 0, 0};
        
        defaultStyle.backgroundColor = {0.2, 0.2, 0.2, 1};
        defaultStyle.background = nullptr;
        add_style("default", defaultStyle);
        add_style("*", Style());

        Style centerStyle;
        centerStyle.horizontalAnchor = CENTER;
        centerStyle.verticalAnchor = MIDDLE;
        add_style("centered", centerStyle);

        Style verticalHandle;
        verticalHandle.horizontalAnchor = CENTER;
        verticalHandle.verticalAnchor = BOTTOM;
        add_style("vertical-handle", verticalHandle);

        Style joystickHandleStyle;
        joystickHandleStyle.horizontalAnchor = CENTER;
        joystickHandleStyle.verticalAnchor = MIDDLE;
        add_style("joystick-handle", joystickHandleStyle);

        Style progressStyle;
        progressStyle.horizontalAnchor = LEFT;
        progressStyle.verticalAnchor = MIDDLE;
        add_style("progress-bar-complete", progressStyle);

        Style scrollHandleStyle;
        scrollHandleStyle.horizontalAnchor = RIGHT;
        scrollHandleStyle.verticalAnchor = TOP;
        add_style("scroll-handle", scrollHandleStyle);

        Style scrollItem;
        scrollItem.margin = {3, 3, 6, 3};
        add_style("scroll-item", scrollItem);
    }

    void Stylesheet::add_style(const std::string& name, const Style& style){
        m_classMap[name] = style;
    }

    void Stylesheet::remove_style(const std::string& name){
        auto iter = m_classMap.find(name);
        assert(iter != m_classMap.end() && "Style doesnt exist on this stylesheet");
        m_classMap.erase(iter);
    }

    Style Stylesheet::get_style(const std::string& identifiers) const {
        // Split identifiers up
        std::vector<std::string> ids = explode(identifiers);

        // Prep default
        Style style = m_classMap.at("default");

        // Cascade style starting with default, *, then any user defined
        style = m_classMap.at("*");

        for(auto& id : ids){
            auto iter = m_classMap.find(id);

            if(iter == m_classMap.end())
                // Skip if this style wasnt found
                continue;

            style = iter->second;
        }

        // Return the finalized style
        return style;
    }
};