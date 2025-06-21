#include <memory>

#include "draft/util/time.hpp"
#include "SFML/System/Time.hpp"

namespace Draft {
    // pImpl declaration
    struct Time::Impl {
        // Variables
        sf::Time time;
    };

    // Constructors
    Time::Time() : ptr(std::make_unique<Impl>()) {}
    Time::Time(const Time& other) : ptr(std::make_unique<Impl>(other.ptr->time)) {}
    Time::Time(Impl time) : ptr(std::make_unique<Impl>(time)) {}
    Time::~Time(){}

    // Functions
    float Time::as_seconds() const { return ptr->time.asSeconds(); }
    int32_t Time::as_milliseconds() const { return ptr->time.asMilliseconds(); }
    int64_t Time::as_microseconds() const { return ptr->time.asMicroseconds(); }

    // Static functions
    Time Time::seconds(float amount){ return Time({ sf::seconds(amount) }); }
    Time Time::milliseconds(int32_t amount){ return Time({ sf::milliseconds(amount) }); }
    Time Time::microseconds(int64_t amount){ return Time({ sf::microseconds(amount) }); }
    
    // Operators
    void Time::operator=(const Time& right){ ptr->time = right.ptr->time; }
    bool Time::operator==(const Time& right){ return (ptr->time == right.ptr->time); }
    bool Time::operator!=(const Time& right){ return (ptr->time != right.ptr->time); }
    bool Time::operator< (const Time& right){ return (ptr->time < right.ptr->time); }
    bool Time::operator> (const Time& right){ return (ptr->time > right.ptr->time); }
    bool Time::operator<=(const Time& right){ return (ptr->time <= right.ptr->time); }
    bool Time::operator>=(const Time& right){ return (ptr->time >= right.ptr->time); }
    Time Time::operator- (){ return Time({ -ptr->time }); }
    Time Time::operator+ (const Time& right){ return Time({ ptr->time + right.ptr->time }); }
    Time& Time::operator+=(const Time& right){ ptr->time += right.ptr->time; return *this; }
    Time Time::operator- (const Time& right){ return Time({ ptr->time - right.ptr->time }); }
    Time& Time::operator-=(const Time& right){ ptr->time -= right.ptr->time; return *this; }
    Time Time::operator* (float right){ return Time({ ptr->time * right }); }
    Time Time::operator* (int64_t right){ return Time({ ptr->time * (long long)right }); }
    Time& Time::operator*=(float right){ ptr->time *= right; return *this; }
    Time& Time::operator*=(int64_t right){ ptr->time *= (long long)right; return *this; }
    Time Time::operator/ (float right){ return Time({ ptr->time / right }); }
    Time Time::operator/ (int64_t right){ return Time({ ptr->time / (long long)right }); }
    Time& Time::operator/=(float right){ ptr->time /= right; return *this; }
    Time& Time::operator/=(int64_t right){ ptr->time /= (long long)right; return *this; }
    float Time::operator/ (const Time& right){ return (ptr->time / right.ptr->time); }
    Time Time::operator% (const Time& right){ return Time({ ptr->time % right.ptr->time }); }
    Time& Time::operator%=(const Time& right){ ptr->time %= right.ptr->time; return *this; }

    // Literals
    Time operator""_s(long double x){ return Time::seconds(x); }
    Time operator""_ms(unsigned long long x){ return Time::milliseconds(x); }
    Time operator""_mis(unsigned long long x){ return Time::microseconds(x); }
};