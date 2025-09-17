#pragma once
#include <optional>

// Idea is to treat UnitValue as an imaginary number, allowing
// for calc() and other functions.
struct UnitValue {
private:
    // Variables
    float pixels = 0.f;
    float percent = 0.f;
    bool m_auto = false;

public:
    // Public statics
    static const UnitValue Auto;

    // Constructors
    UnitValue(float pixels = 0.f, float percent = 0.f);
    UnitValue(bool p_auto);

    // Operators
    UnitValue& operator+();
    UnitValue& operator-();

    UnitValue operator+(const UnitValue& other) const;
    UnitValue operator-(const UnitValue& other) const;
    UnitValue operator*(const UnitValue& other) const;
    UnitValue operator/(const UnitValue& other) const;

    UnitValue& operator+=(const UnitValue& other);
    UnitValue& operator-=(const UnitValue& other);
    UnitValue& operator*=(const UnitValue& other);
    UnitValue& operator/=(const UnitValue& other);

    // Functions
    float get(float max) const;
    bool is_auto() const;
    bool is_pixel() const;
    bool is_percent() const;
};

typedef std::optional<UnitValue> OptUnitValue;

UnitValue operator""_pixels(long double value);
UnitValue operator""_pixels(unsigned long long value);
UnitValue operator""_percent(long double value);
UnitValue operator""_percent(unsigned long long value);