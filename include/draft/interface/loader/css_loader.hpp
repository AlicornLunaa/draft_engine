#pragma once

#include "draft/util/file_handle.hpp"
#include "draft/interface/v2/style.hpp"

namespace CSSLoader {
    void parse_file(Stylesheet& style, const Draft::FileHandle& handle);
};