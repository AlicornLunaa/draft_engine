#pragma once

#include "draft/interface/widgets/layout.hpp"
#include "draft/interface/metrics.hpp"

namespace Draft {
    namespace UI {
        struct DomTree {
            const Layout* layout = nullptr;
            std::vector<DomTree> leafs;
            Metrics metrics{};
        };
    };
};