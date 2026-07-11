#pragma once

#include <chrono>
#include <cstdint>

namespace Draft {
    /**
     * @brief Represents a span of time, storable and comparable independent of any particular unit.
     * Internally stored as whole microseconds via std::chrono.
     */
    class Time {
    public:
        /**
         * @brief Constructs a zero-length Time.
         */
        Time() = default;
        Time(const Time& other) = default;
        ~Time() = default;

        /**
         * @brief Gets this Time as a fractional number of seconds.
         */
        float as_seconds() const;

        /**
         * @brief Gets this Time as a whole number of milliseconds (truncated).
         */
        int32_t as_milliseconds() const;

        /**
         * @brief Gets this Time as a whole number of microseconds.
         */
        int64_t as_microseconds() const;

        /**
         * @brief Creates a Time from a fractional number of seconds.
         */
        static Time seconds(float amount);

        /**
         * @brief Creates a Time from a whole number of milliseconds.
         */
        static Time milliseconds(int32_t amount);

        /**
         * @brief Creates a Time from a whole number of microseconds.
         */
        static Time microseconds(int64_t amount);

        // Operators
        void operator=(const Time& right);
        bool operator==(const Time& right) const;
        bool operator!=(const Time& right) const;
        bool operator< (const Time& right) const;
        bool operator> (const Time& right) const;
        bool operator<=(const Time& right) const;
        bool operator>=(const Time& right) const;
        Time operator- () const;
        Time operator+ (const Time& right) const;
        Time& operator+=(const Time& right);
        Time operator- (const Time& right) const;
        Time& operator-=(const Time& right);
        Time operator* (float right) const;
        Time operator* (int64_t right) const;
        Time& operator*=(float right);
        Time& operator*=(int64_t right);
        Time operator/ (float right) const;
        Time operator/ (int64_t right) const;
        Time& operator/=(float right);
        Time& operator/=(int64_t right);

        /**
         * @brief Ratio of this Time to @p right (e.g. 2.5 if this is 2.5x as long as @p right).
         */
        float operator/ (const Time& right) const;
        Time operator% (const Time& right) const;
        Time& operator%=(const Time& right);

    private:
        std::chrono::microseconds m_duration{0};

        explicit Time(std::chrono::microseconds duration) : m_duration(duration) {}
    };

    /**
     * @brief Literal suffix for seconds, e.g. `1.5_s`.
     */
    Time operator""_s(long double x);

    /**
     * @brief Literal suffix for milliseconds, e.g. `500_ms`.
     */
    Time operator""_ms(unsigned long long x);

    /**
     * @brief Literal suffix for microseconds, e.g. `500_mis`.
     */
    Time operator""_mis(unsigned long long x);
}
