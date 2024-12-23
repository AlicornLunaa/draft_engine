#include "draft/interface/unit_value.hpp"

// Constructors
UnitValue::UnitValue(float pixels, float percent) : pixels(pixels), percent(percent) {
}

// Functions
float UnitValue::get(float max) const {
    return percent * 0.01f * max + pixels;
}

// Literals
UnitValue operator"" _pixels(long double value){ return { static_cast<float>(value), 0.f}; }
UnitValue operator"" _pixels(unsigned long long value){ return { static_cast<float>(value), 0.f}; }
UnitValue operator"" _percent(long double value){ return { 0.f, static_cast<float>(value)}; }
UnitValue operator"" _percent(unsigned long long value){ return { 0.f, static_cast<float>(value)}; }