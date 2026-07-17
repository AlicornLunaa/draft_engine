#include "draft/util/localization.hpp"
#include "draft/util/json.hpp"
#include <stdexcept>

namespace Draft {
    Localization::Localization(const std::string& fallbackLanguage) : m_fallbackLanguage(fallbackLanguage) {
    }

    void Localization::load_language(FileHandle handle){
        std::string language = handle.stem();
        auto languageData = JSON::parse(handle.read_string());
        auto& languageMap = m_languageDb[language];

        for(const auto& [identifier, content] : languageData.items()){
            languageMap[identifier] = content;
        }
    }

    void Localization::unload_language(const std::string& language){
        if(m_languageDb.erase(language) == 0){
            throw std::logic_error("Localization::unload_language(...): Language does not exist");
        }
    }
}