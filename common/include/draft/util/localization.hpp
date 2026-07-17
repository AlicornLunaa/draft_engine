#pragma once

#include "draft/util/files/file_handle.hpp"
#include <format>
#include <string>
#include <unordered_map>

namespace Draft {
    /**
     * @brief Used for localization of strings. Should be structured in the project as
     * assets/localization/en-us.lang, etc. This would allow the asset manager to load
     * the localization by calling assets.get<Localization>("assets/localization") which
     * would load all the JSON files in the folder as languages.
     */
    class Localization {
    public:
        /**
         * @brief Construct a new Localization object
         * @param fallbackLanguage the language to fall-back to if no language is found for it
         */
        Localization(const std::string& fallbackLanguage = "en-us");
        ~Localization() = default;

        /**
         * @brief Loads the language into the database
         * @param handle 
         */
        void load_language(FileHandle handle);

        /**
         * @brief Removes the language from the database by identifier
         * @param language 
         */
        void unload_language(const std::string& language);

        /**
         * @brief Get the content of an identifier
         * 
         * @tparam Args 
         * @param language Language pack to search
         * @param identifier Identifier string to find
         * @param args Any arguments to feed into std::format
         * @return std::string 
         */
        template<typename ...Args>
        std::string get_content(const std::string& language, const std::string& identifier, Args&&... args) const {
            const auto& languageMap = m_languageDb.at(language);
            const auto& content = languageMap.at(identifier);
            return std::vformat(content, std::make_format_args(args...));
        }

    private:
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_languageDb;
        std::string m_fallbackLanguage;
    };
}