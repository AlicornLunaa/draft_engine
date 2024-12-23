#pragma once
#include <optional>

// Idea is to treat UnitValue as an imaginary number, allowing
// for calc() and other functions.
struct UnitValue {
private:
    float pixels = 0.f;
    float percent = 0.f;

public:
    UnitValue(float pixels = 0.f, float percent = 0.f);

    float get(float max) const;
};

typedef std::optional<UnitValue> OptUnitValue;

UnitValue operator"" _pixels(long double value);
UnitValue operator"" _pixels(unsigned long long value);
UnitValue operator"" _percent(long double value);
UnitValue operator"" _percent(unsigned long long value);