#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <cmath>
#include <iostream>

//------------------------------------------------------------------------------
class Angle
{
	static constexpr float kPI = 3.14159265358979323846f;

public:
    // Factory methods to create Angle instances
    static Angle Degrees(float deg) { return Angle(deg * (kPI / 180.0f)); }
    static Angle Radians(float rad) { return Angle(rad); }

    // Conversion functions
    float AsDegrees() const { return mRadians * (180.0f / kPI); }
    float AsRadians() const { return mRadians; }

    // Arithmetic operations
    Angle operator+(const Angle& other) const { return Angle(mRadians + other.mRadians); }
    Angle operator-(const Angle& other) const { return Angle(mRadians - other.mRadians); }
    Angle operator*(float scalar) const { return Angle(mRadians * scalar); }
    Angle operator/(float scalar) const { return Angle(mRadians / scalar); }

    Angle& operator+=(const Angle& other)
    {
        mRadians += other.mRadians;
        return *this;
    }

    Angle& operator-=(const Angle& other)
    {
        mRadians -= other.mRadians;
        return *this;
    }

    Angle& operator*=(float scalar)
    {
        mRadians *= scalar;
        return *this;
    }

    Angle& operator/=(float scalar)
    {
        mRadians /= scalar;
        return *this;
    }

    // Comparison operators
    bool operator==(const Angle& other) const { return mRadians == other.mRadians; }
    bool operator!=(const Angle& other) const { return mRadians != other.mRadians; }
    bool operator<(const Angle& other) const { return mRadians < other.mRadians; }
    bool operator>(const Angle& other) const { return mRadians > other.mRadians; }
    bool operator<=(const Angle& other) const { return mRadians <= other.mRadians; }
    bool operator>=(const Angle& other) const { return mRadians >= other.mRadians; }

    // Negation
    Angle operator-() const { return Angle(-mRadians); }

    // Wrapping functions
    Angle WrapSigned() const
    {
        float wrapped = std::fmod(mRadians + kPI, 2 * kPI);
        if (wrapped < 0) { wrapped += 2 * kPI; }
        return Angle(wrapped - kPI);
    }

    Angle WrapUnsigned() const
    {
        float wrapped = std::fmod(mRadians, 2 * kPI);
        if (wrapped < 0) { wrapped += 2 * kPI; }
        return Angle(wrapped);
    }

private:
    explicit Angle(float rad) 
        : mRadians(rad) 
    { }

    float mRadians;
};

// User-defined literals for convenient angle creation
//------------------------------------------------------------------------------
namespace AngleLiterals
{
    Angle operator"" _deg(long double deg);
    Angle operator"" _rad(long double rad);
}