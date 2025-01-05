#pragma once

#include "draft/util/file_handle.hpp"

namespace Draft {
    namespace UI {
        class Stylesheet;
        
        namespace CSSLoader {
            enum PseudoSelector { NONE, HOVER, PSEUDO_COUNT };
            enum class CSSPosition { ABSOLUTE, RELATIVE, FIXED, STATIC };
            enum class CSSDisplay { BLOCK, INLINE, NONE };
            enum class CSSBoxSizing { CONTENT_BOX, BORDER_BOX };

            void parse_file(Stylesheet& style, const Draft::FileHandle& handle);
        };
    };
};