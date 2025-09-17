#include "draft/interface/unit_value.hpp"
#include <cassert>

// Statics
const UnitValue UnitValue::Auto = UnitValue(true);

// Constructors
UnitValue::UnitValue(float pixels, float percent) : pixels(pixels), percent(percent), m_auto(false) {}
UnitValue::UnitValue(bool p_auto) : pixels(0), percent(0), m_auto(p_auto) {}

// Operators
UnitValue& UnitValue::operator+(){ return *this; }

UnitValue& UnitValue::operator-(){
    pixels *= -1;
    percent *= -1;
    return *this;
}

UnitValue UnitValue::operator+(const UnitValue& other) const {
    UnitValue val(this->pixels + other.pixels, this->percent + other.percent);
    val.m_auto |= other.m_auto;
    val.m_auto |= m_auto;
    return val;
}

UnitValue UnitValue::operator-(const UnitValue& other) const {
    UnitValue val(this->pixels - other.pixels, this->percent - other.percent);
    val.m_auto |= other.m_auto;
    val.m_auto |= m_auto;
    return val;
}

UnitValue UnitValue::operator*(const UnitValue& other) const {
    UnitValue val(this->pixels * other.pixels, this->percent * other.percent);
    val.m_auto |= other.m_auto;
    val.m_auto |= m_auto;
    return val;
}

UnitValue UnitValue::operator/(const UnitValue& other) const {
    UnitValue val(this->pixels / other.pixels, this->percent / other.percent);
    val.m_auto |= other.m_auto;
    val.m_auto |= m_auto;
    return val;
}

UnitValue& UnitValue::operator+=(const UnitValue& other){
    this->pixels += other.pixels;
    this->percent += other.percent;
    m_auto |= other.m_auto;
    return *this;
}

UnitValue& UnitValue::operator-=(const UnitValue& other){
    this->pixels -= other.pixels;
    this->percent -= other.percent;
    m_auto |= other.m_auto;
    return *this;
}

UnitValue& UnitValue::operator*=(const UnitValue& other){
    this->pixels *= other.pixels;
    this->percent *= other.percent;
    m_auto |= other.m_auto;
    return *this;
}

UnitValue& UnitValue::operator/=(const UnitValue& other){
    this->pixels /= other.pixels;
    this->percent /= other.percent;
    m_auto |= other.m_auto;
    return *this;
}

// Functions
float UnitValue::get(float max) const { return percent * 0.01f * max + pixels; }
bool UnitValue::is_auto() const { return m_auto; }
bool UnitValue::is_pixel() const { return pixels != 0.f; }
bool UnitValue::is_percent() const { return percent != 0.f; }

// Literals
UnitValue operator""_pixels(long double value){ return { static_cast<float>(value), 0.f}; }
UnitValue operator""_pixels(unsigned long long value){ return { static_cast<float>(value), 0.f}; }
UnitValue operator""_percent(long double value){ return { 0.f, static_cast<float>(value)}; }
UnitValue operator""_percent(unsigned long long value){ return { 0.f, static_cast<float>(value)}; }