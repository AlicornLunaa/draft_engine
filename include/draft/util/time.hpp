#pragma once

#include <cstdint>
#include <memory>

namespace Draft {
    class Time {
    public:
        // Constructors
        Time();
        Time(const Time& other);
        ~Time();

        // Functions
        float as_seconds() const;
        int32_t as_milliseconds() const;
        int64_t as_microseconds() const;

        // Static functions
        static Time seconds(float amount);
        static Time milliseconds(int32_t amount);
        static Time microseconds(int64_t amount);

        // Operators
        void operator=(const Time& right);
        bool operator==(const Time& right);
        bool operator!=(const Time& right);
        bool operator< (const Time& right);
        bool operator> (const Time& right);
        bool operator<=(const Time& right);
        bool operator>=(const Time& right);
        Time operator- ();
        Time operator+ (const Time& right);
        Time& operator+=(const Time& right);
        Time operator- (const Time& right);
        Time& operator-=(const Time& right);
        Time operator* (float right);
        Time operator* (int64_t right);
        Time& operator*=(float right);
        Time& operator*=(int64_t right);
        Time operator/ (float right);
        Time operator/ (int64_t right);
        Time& operator/=(float right);
        Time& operator/=(int64_t right);
        float operator/ (const Time& right);
        Time operator% (const Time& right);
        Time& operator%=(const Time& right);

    private:
        // PimPL!
        struct Impl;
        std::unique_ptr<Impl> ptr;

        // Internal constructor
        explicit Time(Impl time);
    };

    Time operator""_s(long double x);
    Time operator""_ms(unsigned long long x);
    Time operator""_mis(unsigned long long x);
};