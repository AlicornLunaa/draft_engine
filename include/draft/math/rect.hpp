#pragma once

namespace Draft {
    template<typename T>
    struct Rect {
        // Variables
        T x;
        T y;
        T width;
        T height;
    };

    typedef Rect<int> IntRect;
    typedef Rect<float> FloatRect;
    typedef Rect<double> DoubleRect;
};