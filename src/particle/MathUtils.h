#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;

    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const {
        return {x + o.x, y + o.y, z + o.z};
    }

    Vec3 operator-(const Vec3& o) const {
        return {x - o.x, y - o.y, z - o.z};
    }

    Vec3 operator*(float s) const {
        return {x * s, y * s, z * s};
    }

    Vec3& operator+=(const Vec3& o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }

    float len() const {
        return sqrtf(x * x + y * y + z * z);
    }

    Vec3 norm() const {
        float l = len();
        if (l > 0) {
            return (*this) * (1.f / l);
        } else {
            return Vec3();
        }
    }
};

struct Color4 {
    float r, g, b, a;

    Color4(float r = 1, float g = 1, float b = 1, float a = 1) : r(r), g(g), b(b), a(a) {}

    Color4 lerp(const Color4& o, float t) const {
        return {
            r + (o.r - r) * t,
            g + (o.g - g) * t,
            b + (o.b - b) * t,
            a + (o.a - a) * t
        };
    }
};

inline float randf(float lo, float hi) {
    return lo + (hi - lo) * (rand() / (float)RAND_MAX);
}

inline float randf01() {
    return rand() / (float)RAND_MAX;
}
