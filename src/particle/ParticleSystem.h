#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include "Particle.h"

enum EmitterType {
    EMITTER_FIRE,
    EMITTER_EXPLOSION,
    EMITTER_FOUNTAIN,
    EMITTER_GALAXY,
    EMITTER_SNOW,
    EMITTER_COUNT
};

static const char* emitterNames[] = {
    "Fire", "Explosion", "Fountain", "Galaxy", "Snow"
};

class ParticleSystem {
public:
    static const int MAX_PARTICLES = 8000;

    Particle pool[MAX_PARTICLES];
    int      poolHead = 0;

    EmitterType type      = EMITTER_FIRE;
    Vec3        emitPos   = {0, 0, 0};
    float       emitTimer = 0.f;
    float       emitRate  = 200.f;
    bool        burst     = false;
    bool        active    = true;

    float camYaw   = 30.f;
    float camPitch = 20.f;
    float camDist  = 18.f;

    int   aliveCount = 0;
    float totalTime  = 0.f;

    void init() {
        srand((unsigned)time(nullptr));
        for (auto& p : pool) {
            p.alive = false;
        }
    }

    void switchEmitter(EmitterType t) {
        type = t;
        burst = false;
        emitTimer = 0.f;
        for (auto& p : pool) {
            p.alive = false;
        }
    }

    void update(float dt) {
        totalTime += dt;

        if (active) {
            switch (type) {
                case EMITTER_FIRE: {
                    emitTimer += dt;
                    int n = (int)(emitTimer * emitRate);
                    if (n > 0) {
                        spawnFire(n);
                        emitTimer -= n / emitRate;
                    }
                } break;
                case EMITTER_EXPLOSION: {
                    if (!burst) {
                        spawnExplosion();
                        burst = true;
                    }
                } break;
                case EMITTER_FOUNTAIN: {
                    emitTimer += dt;
                    int n = (int)(emitTimer * 150.f);
                    if (n > 0) {
                        spawnFountain(n);
                        emitTimer -= n / 150.f;
                    }
                } break;
                case EMITTER_GALAXY: {
                    emitTimer += dt;
                    int n = (int)(emitTimer * 80.f);
                    if (n > 0) {
                        spawnGalaxy(n);
                        emitTimer -= n / 80.f;
                    }
                } break;
                case EMITTER_SNOW: {
                    emitTimer += dt;
                    int n = (int)(emitTimer * 60.f);
                    if (n > 0) {
                        spawnSnow(n);
                        emitTimer -= n / 60.f;
                    }
                } break;
                default: break;
            }
        }

        aliveCount = 0;
        for (auto& p : pool) {
            if (!p.alive) {
                continue;
            }
            p.vel += p.acc * dt;
            p.pos += p.vel * dt;
            p.life -= dt;
            if (p.life <= 0) {
                p.alive = false;
                continue;
            }
            aliveCount++;
        }
    }

    void render() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glDisable(GL_LIGHTING);

        float mv[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, mv);
        Vec3 right(mv[0], mv[4], mv[8]);
        Vec3 up   (mv[1], mv[5], mv[9]);

        glBegin(GL_QUADS);
        for (auto& p : pool) {
            if (!p.alive) {
                continue;
            }

            float t = 1.f - p.life / p.maxLife;

            Color4 c = p.colorStart.lerp(p.colorEnd, t);
            float  s = p.size + (p.sizeEnd - p.size) * t;

            glColor4f(c.r, c.g, c.b, c.a);

            Vec3 r = right * s, u = up * s;
            Vec3 bl = p.pos - r - u;
            Vec3 br = p.pos + r - u;
            Vec3 tr = p.pos + r + u;
            Vec3 tl = p.pos - r + u;

            glTexCoord2f(0, 0); glVertex3f(bl.x, bl.y, bl.z);
            glTexCoord2f(1, 0); glVertex3f(br.x, br.y, br.z);
            glTexCoord2f(1, 1); glVertex3f(tr.x, tr.y, tr.z);
            glTexCoord2f(0, 1); glVertex3f(tl.x, tl.y, tl.z);
        }
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

private:

    Particle* alloc() {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            int idx = (poolHead + i) % MAX_PARTICLES;
            if (!pool[idx].alive) {
                poolHead = (idx + 1) % MAX_PARTICLES;
                return &pool[idx];
            }
        }

        poolHead = (poolHead + 1) % MAX_PARTICLES;
        return &pool[(poolHead - 1 + MAX_PARTICLES) % MAX_PARTICLES];
    }

    void spawnFire(int n) {
        for (int i = 0; i < n; i++) {
            Particle* p = alloc();
            float angle = randf(0, 2.f * (float)M_PI);
            float r = randf(0, 0.4f);
            p->pos = emitPos + Vec3(cosf(angle) * r, 0, sinf(angle) * r);
            p->vel = Vec3(randf(-0.3f, 0.3f), randf(2.f, 5.f), randf(-0.3f, 0.3f));
            p->acc = Vec3(0, -0.5f, 0);
            p->colorStart = Color4(1.f, randf(0.3f, 0.7f), 0.f, 1.f);
            p->colorEnd   = Color4(0.8f, 0.1f, 0.f, 0.f);
            p->size = randf(0.15f, 0.35f);
            p->sizeEnd = 0.02f;
            p->maxLife = randf(0.8f, 2.0f);
            p->life = p->maxLife;
            p->alive = true;
        }
    }

    void spawnExplosion() {
        int n = 600;
        for (int i = 0; i < n; i++) {
            Particle* p = alloc();
            float phi = randf(0, 2.f * (float)M_PI);
            float theta = acosf(randf(-1, 1));
            float speed = randf(2.f, 9.f);
            Vec3 dir(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));
            p->pos = emitPos;
            p->vel = dir * speed;
            p->acc = Vec3(0, -4.f, 0);

            float hue = randf01();
            float h6 = hue * 6.f;
            int hi = (int)h6;
            float f = h6 - hi;
            float q = 1 - f, t2 = f;
            Color4 cs;
            switch (hi % 6) {
                case 0: cs = {1, t2, 0, 1}; break;
                case 1: cs = {q, 1, 0, 1};  break;
                case 2: cs = {0, 1, t2, 1}; break;
                case 3: cs = {0, q, 1, 1};  break;
                case 4: cs = {t2, 0, 1, 1}; break;
                default: cs = {1, 0, q, 1}; break;
            }
            p->colorStart = cs;
            p->colorEnd = Color4(cs.r * 0.3f, cs.g * 0.3f, cs.b * 0.3f, 0.f);
            p->size = randf(0.08f, 0.22f);
            p->sizeEnd = 0.01f;
            p->maxLife = randf(1.0f, 2.5f);
            p->life = p->maxLife;
            p->alive = true;
        }
    }

    void spawnFountain(int n) {
        for (int i = 0; i < n; i++) {
            Particle* p = alloc();
            float angle = randf(0, 2.f * (float)M_PI);
            float spread = randf(0, 0.4f);
            p->pos = emitPos;
            p->vel = Vec3(cosf(angle) * spread, randf(6.f, 10.f), sinf(angle) * spread);
            p->acc = Vec3(0, -9.8f, 0);
            float t = randf01();
            p->colorStart = Color4(0.2f + t * 0.3f, 0.5f + t * 0.5f, 1.f, 1.f);
            p->colorEnd   = Color4(0.f, 0.3f, 1.f, 0.f);
            p->size = randf(0.06f, 0.14f);
            p->sizeEnd = 0.02f;
            p->maxLife = randf(1.5f, 3.0f);
            p->life = p->maxLife;
            p->alive = true;
        }
    }

    void spawnGalaxy(int n) {
        for (int i = 0; i < n; i++) {
            Particle* p = alloc();
            float arm = randf(0, 2.f * (float)M_PI);
            float radius = randf(0.1f, 5.f);
            float twist = radius * 0.8f;
            float ax = cosf(arm + twist) * radius + randf(-0.3f, 0.3f);
            float az = sinf(arm + twist) * radius + randf(-0.3f, 0.3f);
            p->pos = emitPos + Vec3(ax, randf(-0.1f, 0.1f), az);
            float speed = sqrtf(radius) * 0.8f;
            Vec3 tangent(-sinf(arm + twist), 0, cosf(arm + twist));
            p->vel = tangent * speed + Vec3(0, randf(-0.05f, 0.05f), 0);
            p->acc = Vec3(0, 0, 0);
            float t = radius / 5.f;
            p->colorStart = Color4(0.4f + t * 0.6f, 0.6f - t * 0.3f, 1.f - t * 0.5f, 1.f);
            p->colorEnd   = Color4(1.f, 0.8f, 0.2f, 0.f);
            p->size = randf(0.04f, 0.12f);
            p->sizeEnd = 0.01f;
            p->maxLife = randf(3.f, 6.f);
            p->life = p->maxLife;
            p->alive = true;
        }
    }

    void spawnSnow(int n) {
        for (int i = 0; i < n; i++) {
            Particle* p = alloc();
            p->pos = emitPos + Vec3(randf(-8, 8), randf(6, 10), randf(-8, 8));
            p->vel = Vec3(randf(-0.3f, 0.3f), randf(-1.5f, -0.5f), randf(-0.3f, 0.3f));
            p->acc = Vec3(0, 0, 0);
            float w = randf(0.85f, 1.f);
            p->colorStart = Color4(w, w, 1.f, 0.9f);
            p->colorEnd   = Color4(0.7f, 0.7f, 1.f, 0.f);
            p->size = randf(0.05f, 0.15f);
            p->sizeEnd = randf(0.02f, 0.08f);
            p->maxLife = randf(4.f, 8.f);
            p->life = p->maxLife;
            p->alive = true;
        }
    }
};
