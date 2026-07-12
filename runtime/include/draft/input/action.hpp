#pragma once

namespace Draft {
    /**
     * @brief The three states a key/button can report on a given callback.
     */
    enum Action {
        PRESS = 1,
        RELEASE = 0,
        HOLD = 2
    };
}
