//
// Copyright (c) 2008-2018 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// IntVector3 modified to use 64 bit ints
// This is a pretty substantial part of Urho3D

#pragma once


#include <cstdint>
#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Math/MathDefs.h>

#include "LongVector3.h"

namespace Urho3D
{

/// Three-dimensional vector with integer values.
class URHO3D_API LongVector3
{
public:
    /// Construct a zero vector.
    LongVector3() noexcept :
        x_(0),
        y_(0),
        z_(0)
    {
    }

    /// Construct from coordinates.
    LongVector3(int64_t x, int64_t y, int64_t z) noexcept :
        x_(x),
        y_(y),
        z_(z)
    {
    }

    /// Construct from an int array.
    explicit LongVector3(const int64_t* data) noexcept :
        x_(data[0]),
        y_(data[1]),
        z_(data[2])
    {
    }

    /// Copy-construct from another vector.
    LongVector3(const LongVector3& rhs) noexcept = default;

    /// Assign from another vector.
    LongVector3& operator =(const LongVector3& rhs) noexcept = default;

    /// Test for equality with another vector.
    bool operator ==(const LongVector3& rhs) const { return x_ == rhs.x_ && y_ == rhs.y_ && z_ == rhs.z_; }

    /// Test for inequality with another vector.
    bool operator !=(const LongVector3& rhs) const { return x_ != rhs.x_ || y_ != rhs.y_ || z_ != rhs.z_; }

    /// Add a vector.
    LongVector3 operator +(const LongVector3& rhs) const { return LongVector3(x_ + rhs.x_, y_ + rhs.y_, z_ + rhs.z_); }

    /// Return negation.
    LongVector3 operator -() const { return LongVector3(-x_, -y_, -z_); }

    /// Subtract a vector.
    LongVector3 operator -(const LongVector3& rhs) const { return LongVector3(x_ - rhs.x_, y_ - rhs.y_, z_ - rhs.z_); }

    /// Multiply with a scalar.
    LongVector3 operator *(int64_t rhs) const { return LongVector3(x_ * rhs, y_ * rhs, z_ * rhs); }

    /// Multiply with a vector.
    LongVector3 operator *(const LongVector3& rhs) const { return LongVector3(x_ * rhs.x_, y_ * rhs.y_, z_ * rhs.z_); }

    /// Divide by a scalar.
    LongVector3 operator /(int64_t rhs) const { return LongVector3(x_ / rhs, y_ / rhs, z_ / rhs); }

    /// Divide by a vector.
    LongVector3 operator /(const LongVector3& rhs) const { return LongVector3(x_ / rhs.x_, y_ / rhs.y_, z_ / rhs.z_); }

    /// Add-assign a vector.
    LongVector3& operator +=(const LongVector3& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    /// Subtract-assign a vector.
    LongVector3& operator -=(const LongVector3& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        return *this;
    }

    /// Multiply-assign a scalar.
    LongVector3& operator *=(int64_t rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        return *this;
    }

    /// Multiply-assign a vector.
    LongVector3& operator *=(const LongVector3& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        z_ *= rhs.z_;
        return *this;
    }

    /// Divide-assign a scalar.
    LongVector3& operator /=(int rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        z_ /= rhs;
        return *this;
    }

    /// Divide-assign a vector.
    LongVector3& operator /=(const LongVector3& rhs)
    {
        x_ /= rhs.x_;
        y_ /= rhs.y_;
        z_ /= rhs.z_;
        return *this;
    }

    /// Return integer data.
    const int64_t* Data() const { return &x_; }

    /// Return as string.
    String ToString() const;

    /// Return hash value for HashSet & HashMap.
    unsigned ToHash() const { return (unsigned)x_ * 31 * 31 + (unsigned)y_ * 31 + (unsigned)z_; }

    /// Return length.
    float Length() const { return sqrtf((float)(x_ * x_ + y_ * y_ + z_ * z_)); }

    /// X coordinate.
    int64_t x_;
    /// Y coordinate.
    int64_t y_;
    /// Z coordinate.
    int64_t z_;

    /// Zero vector.
    static const LongVector3 ZERO;
    /// (-1,0,0) vector.
    static const LongVector3 LEFT;
    /// (1,0,0) vector.
    static const LongVector3 RIGHT;
    /// (0,1,0) vector.
    static const LongVector3 UP;
    /// (0,-1,0) vector.
    static const LongVector3 DOWN;
    /// (0,0,1) vector.
    static const LongVector3 FORWARD;
    /// (0,0,-1) vector.
    static const LongVector3 BACK;
    /// (1,1,1) vector.
    static const LongVector3 ONE;
};

}
