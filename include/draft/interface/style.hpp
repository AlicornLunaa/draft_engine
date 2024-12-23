#pragma once

#include "draft/interface/widgets/layout.hpp"
#include "draft/interface/enums.hpp"

#include <any>
#include <array>
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>

struct Rule {
    std::vector<std::string> selectors; // Usually will be 1 element but might be more if parents are defined like '.side-bar > button {}'
    PseudoSelector pseudoSelector;
    std::map<std::string, std::any> properties; // Points Property -> value
};

class Stylesheet {
private:
    std::array<std::unordered_map<std::string, Rule>, PSEUDO_COUNT> styles; // Points selector -> rules, where each index is a different pseudo selector

    void recursive_class_concat(std::vector<std::string>& allClasses, Layout* ptr) const {
        if(!ptr)
            // Basecase
            return;

        // Add all the parent classes
        recursive_class_concat(allClasses, ptr->parent);

        // Add all of this layout's classes
        for(auto& _class : ptr->classes){
            allClasses.push_back(_class);
        }
    }

public:
    void clear(){
        for(auto& map : styles){
            map.clear();
        }
    }

    template<typename T>
    std::optional<T> get(Layout& layout, const std::string& property, PseudoSelector pseudo = NONE) const {
        // Calculate all classes and parent classes to get the correct property
        std::vector<std::string> allClasses;
        allClasses.push_back("*");
        allClasses.push_back(layout.elementClass);
        recursive_class_concat(allClasses, &layout);

        std::any const* currentValuePtr = nullptr;

        // With all classes added to the vector, cascade the styles down from beginning to end
        for(const std::string& _class : allClasses){
            // Extract the rule for this type, skip if it doesnt exist
            auto ruleIter = styles[pseudo].find(_class);

            if(ruleIter == styles[pseudo].end())
                continue;
            
            const Rule& rule = ruleIter->second;

            // Now that the rule exists, we need to check for other rules such as a pseudo-selector or descendant selectors
            // Reverse iteration through the selectors and check for parents classes iteratively
            Layout* ptr = layout.parent;
            bool ancestorsExist = !(rule.selectors.size() > 1);

            for(int i = rule.selectors.size() - 2; i >= 0 && ptr; i--){
                const std::string& ancestorClass = rule.selectors[i];

                // Search for parent classes to make sure it has ancestorClass
                bool parentHasAncestor = false;

                for(const std::string& parentClass : ptr->classes){
                    if(parentClass == ancestorClass){
                        // This ancestor exists, move onto the next one
                        ptr = ptr->parent;
                        parentHasAncestor = true;
                        break;
                    }
                }

                if(!parentHasAncestor){
                    // Parent doesnt have this ancestor, quit this immediately
                    break;
                }

                if(i == 0){
                    // This case defines when all ancestors have been accounted for
                    ancestorsExist = true;
                }
            }

            // If ancestors dont exist, quick exit since this rule no longer applies
            if(!ancestorsExist)
                continue;
            
            // With the rule found and existing, extract the property value
            auto propertyIter = rule.properties.find(property);

            if(propertyIter == rule.properties.end())
                continue;

            // Rule and property exist, set this value to current
            currentValuePtr = &propertyIter->second;
        }

        // Return a nullptr if nothing was set
        if(!currentValuePtr)
            return std::nullopt;

        return std::any_cast<T>(*currentValuePtr);
    }

    template<typename T>
    T get(Layout& layout, const std::string& property, T defaultValue, PseudoSelector pseudo = PseudoSelector::NONE) const {
        // This function is a simple helper for the other get function. It will return a default value if no properties were found
        std::optional<T> result = get<T>(layout, property, pseudo);
        return result.value_or(defaultValue);
    }

    template<typename T>
    void set(const std::vector<std::string>& selectors, PseudoSelector pseudoSelector, const std::string& property, const T& value){
        // Sets this css rule
        assert(!selectors.empty() && "Cannot have empty rule");
        std::string mainSelector = selectors.back();

        if(styles[pseudoSelector].find(mainSelector) == styles[pseudoSelector].end()){
            styles[pseudoSelector][mainSelector] = { selectors, pseudoSelector };
        }

        styles[pseudoSelector][mainSelector].properties[property] = value;
    }
};