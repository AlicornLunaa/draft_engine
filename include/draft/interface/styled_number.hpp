#pragma once

namespace Draft {
    namespace UI {
        // Number class for UI, supporting pixel counts or percentages.
        struct StyledNumber {
            float value = 0.f;
            bool percentage = false;

            StyledNumber(float x = 0.f);
            StyledNumber(float x, bool val);

            operator float(){ return value; }

            float calculate(float max = -1.f) const;
        };

        typedef StyledNumber SNumber;
    };
};

Draft::UI::StyledNumber operator"" _pixels(long double x);
Draft::UI::StyledNumber operator"" _percent(long double x);