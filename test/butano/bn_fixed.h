#ifndef BN_FIXED_H
#define BN_FIXED_H

// -------------------------------------------------------------------------
// Host-side shim for Butano's bn::fixed (test builds only).
//
// Butano's bn::fixed is fixed_t<12>: a 32-bit integer storing a value scaled
// by 2^12 (4096). This shim replicates the same fixed-point representation and
// truncation-toward-zero arithmetic so that the real game logic in
// src/entity.cpp and src/ball.cpp compiles and behaves like it does on the GBA,
// without requiring the devkitARM toolchain.
//
// Only the surface area used by the pure-logic source files is implemented.
// -------------------------------------------------------------------------

namespace bn
{

template<int Precision>
class fixed_t
{
public:
    static constexpr int scale = 1 << Precision;

    constexpr fixed_t() = default;
    constexpr fixed_t(int value) : _value(value << Precision) {}
    constexpr fixed_t(double value) : _value(static_cast<int>(value * scale)) {}

    static constexpr fixed_t from_data(int data)
    {
        fixed_t f;
        f._value = data;
        return f;
    }

    [[nodiscard]] constexpr int data() const { return _value; }
    [[nodiscard]] constexpr double to_double() const
    {
        return static_cast<double>(_value) / scale;
    }
    // Explicit so it doesn't clash with the implicit int constructor during
    // overload resolution (e.g. `value < 0`). CHECK_NEAR casts explicitly.
    explicit constexpr operator double() const { return to_double(); }

    constexpr fixed_t operator-() const { return from_data(-_value); }

    constexpr fixed_t operator+(fixed_t o) const { return from_data(_value + o._value); }
    constexpr fixed_t operator-(fixed_t o) const { return from_data(_value - o._value); }
    constexpr fixed_t operator*(fixed_t o) const
    {
        return from_data(static_cast<int>(
            (static_cast<long long>(_value) * o._value) / scale));
    }
    constexpr fixed_t operator/(fixed_t o) const
    {
        return from_data(static_cast<int>(
            (static_cast<long long>(_value) * scale) / o._value));
    }

    constexpr fixed_t& operator+=(fixed_t o) { _value += o._value; return *this; }
    constexpr fixed_t& operator-=(fixed_t o) { _value -= o._value; return *this; }
    constexpr fixed_t& operator*=(fixed_t o) { *this = *this * o; return *this; }
    constexpr fixed_t& operator/=(fixed_t o) { *this = *this / o; return *this; }

    constexpr bool operator==(fixed_t o) const { return _value == o._value; }
    constexpr bool operator!=(fixed_t o) const { return _value != o._value; }
    constexpr bool operator<(fixed_t o)  const { return _value <  o._value; }
    constexpr bool operator>(fixed_t o)  const { return _value >  o._value; }
    constexpr bool operator<=(fixed_t o) const { return _value <= o._value; }
    constexpr bool operator>=(fixed_t o) const { return _value >= o._value; }

private:
    int _value = 0;
};

using fixed = fixed_t<12>;

}

#endif
