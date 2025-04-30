#pragma once

#include <string>

namespace Draft {
    /// Basic tagging component used to labelling
    struct TagComponent {
        std::string tag;
        operator std::string& (){ return tag; }
        operator const std::string& () const { return tag; }
    };
}