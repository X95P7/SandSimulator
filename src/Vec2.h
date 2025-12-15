#pragma once

#include <cmath>

struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator*(double scalar) const { return Vec2(x * static_cast<float>(scalar), y * static_cast<float>(scalar)); }
    Vec2 operator*(int scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }
    Vec2 operator-(Vec2 other) const { return Vec2(x - other.x, y - other.y); }
    float magnitude() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const { return Vec2(x / magnitude(), y / magnitude()); }
};
