#include "draft/util/time.hpp"

namespace Draft {
    // Functions
    float Time::as_seconds() const {
        return std::chrono::duration<float>(m_duration).count();
    }

    int32_t Time::as_milliseconds() const {
        return static_cast<int32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(m_duration).count());
    }

    int64_t Time::as_microseconds() const {
        return m_duration.count();
    }

    // Static functions
    Time Time::seconds(float amount) {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<float>(amount)));
    }

    Time Time::milliseconds(int32_t amount) {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(amount)));
    }

    Time Time::microseconds(int64_t amount) {
        return Time(std::chrono::microseconds(amount));
    }

    // Operators
    void Time::operator=(const Time& right) { m_duration = right.m_duration; }
    bool Time::operator==(const Time& right) const { return m_duration == right.m_duration; }
    bool Time::operator!=(const Time& right) const { return m_duration != right.m_duration; }
    bool Time::operator<(const Time& right) const { return m_duration < right.m_duration; }
    bool Time::operator>(const Time& right) const { return m_duration > right.m_duration; }
    bool Time::operator<=(const Time& right) const { return m_duration <= right.m_duration; }
    bool Time::operator>=(const Time& right) const { return m_duration >= right.m_duration; }

    Time Time::operator-() const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(-m_duration));
    }

    Time Time::operator+(const Time& right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration + right.m_duration));
    }

    Time& Time::operator+=(const Time& right) { m_duration += right.m_duration; return *this; }

    Time Time::operator-(const Time& right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration - right.m_duration));
    }

    Time& Time::operator-=(const Time& right) { m_duration -= right.m_duration; return *this; }

    Time Time::operator*(float right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration * right));
    }

    Time Time::operator*(int64_t right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration * right));
    }

    Time& Time::operator*=(float right) {
        m_duration = std::chrono::duration_cast<std::chrono::microseconds>(m_duration * right);
        return *this;
    }

    Time& Time::operator*=(int64_t right) { m_duration *= right; return *this; }

    Time Time::operator/(float right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration / right));
    }

    Time Time::operator/(int64_t right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration / right));
    }

    Time& Time::operator/=(float right) {
        m_duration = std::chrono::duration_cast<std::chrono::microseconds>(m_duration / right);
        return *this;
    }

    Time& Time::operator/=(int64_t right) { m_duration /= right; return *this; }

    float Time::operator/(const Time& right) const {
        // Deliberately not delegating to duration's own operator/ between two same-rep
        // durations - that performs integer division and would truncate ratios like 1/3.
        return static_cast<float>(m_duration.count()) / static_cast<float>(right.m_duration.count());
    }

    Time Time::operator%(const Time& right) const {
        return Time(std::chrono::duration_cast<std::chrono::microseconds>(m_duration % right.m_duration));
    }

    Time& Time::operator%=(const Time& right) { m_duration %= right.m_duration; return *this; }

    // Literals
    Time operator""_s(long double x) { return Time::seconds(static_cast<float>(x)); }
    Time operator""_ms(unsigned long long x) { return Time::milliseconds(static_cast<int32_t>(x)); }
    Time operator""_mis(unsigned long long x) { return Time::microseconds(static_cast<int64_t>(x)); }
}
