#pragma once
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <string>

static GLuint g_texParticle = 0;

inline void createParticleTexture() {
    const int SZ = 64;
    unsigned char data[SZ * SZ * 4];
    float cx = SZ / 2.f, cy = SZ / 2.f, r = SZ / 2.f;

    for (int y = 0; y < SZ; y++) {
        for (int x = 0; x < SZ; x++) {
            float dx = x - cx, dy = y - cy;
            float d = sqrtf(dx * dx + dy * dy) / r;
            float a = d < 1.f ? (1.f - d * d) * (1.f - d * d) : 0.f;
            int idx = (y * SZ + x) * 4;
            data[idx] = data[idx + 1] = data[idx + 2] = 255;
            data[idx + 3] = (unsigned char)(a * 255);
        }
    }

    glGenTextures(1, &g_texParticle);
    glBindTexture(GL_TEXTURE_2D, g_texParticle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SZ, SZ, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

inline void drawGrid() {
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; i++) {
        float fi = (float)i;
        glColor4f(0.3f, 0.5f, 0.8f, 0.15f);

        glVertex3f(fi, 0, -10);
        glVertex3f(fi, 0, 10);

        glVertex3f(-10, 0, fi);
        glVertex3f(10, 0, fi);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

inline void drawText2D(float x, float y, const std::string& s,
                float r = 1.f, float g = 1.f, float b = 1.f) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : s) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, (int)c);
    }
}
