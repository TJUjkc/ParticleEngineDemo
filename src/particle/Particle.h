#pragma once
#include "MathUtils.h"

struct Particle {
    Vec3   pos;
    Vec3   vel;
    Vec3   acc;

    Color4 colorStart;
    Color4 colorEnd;

    float  size;
    float  sizeEnd;

    float  life;
    float  maxLife;

    bool   alive;
};
