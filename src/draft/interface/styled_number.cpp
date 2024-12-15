#include "draft/interface/styled_number.hpp"

namespace Draft::UI {
    StyledNumber::StyledNumber(float x) : value(x) {
    }

    StyledNumber::StyledNumber(float x, bool val) : value(x), percentage(val) {
    }

    float StyledNumber::calculate(float max) const {
        if(!percentage || max <= 0.f){
            return value;
        } else {
            return max * value;
        }
    }
};

Draft::UI::StyledNumber operator"" _pixels(long double x){ return Draft::UI::StyledNumber{(float)x, false}; }
Draft::UI::StyledNumber operator"" _percent(long double x){ return Draft::UI::StyledNumber{(float)(x / 100), true}; }