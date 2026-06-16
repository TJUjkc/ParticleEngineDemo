#define _USE_MATH_DEFINES
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

#include "particle/ParticleSystem.h"
#include "renderer/Renderer.h"

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

static ParticleSystem g_ps;
static int   g_width = 1280, g_height = 720;
static bool  g_mouseDown = false;
static int   g_mouseX = 0, g_mouseY = 0;
static DWORD g_lastTick = 0;
static bool  g_showHelp = true;
static bool  g_paused = false;
static float g_bgBrightness = 0.f;

void reshape(int w, int h) {
    g_width = w;
    g_height = h;
    glViewport(0, 0, w, h);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case '1': {
            g_ps.switchEmitter(EMITTER_FIRE);
            g_bgBrightness = 0.f;
        } break;
        case '2': {
            g_ps.switchEmitter(EMITTER_EXPLOSION);
            g_bgBrightness = 0.f;
        } break;
        case '3': {
            g_ps.switchEmitter(EMITTER_FOUNTAIN);
            g_bgBrightness = 0.02f;
        } break;
        case '4': {
            g_ps.switchEmitter(EMITTER_GALAXY);
            g_bgBrightness = 0.f;
        } break;
        case '5': {
            g_ps.switchEmitter(EMITTER_SNOW);
            g_bgBrightness = 0.04f;
        } break;
        case 'r': case 'R': {
            g_ps.switchEmitter(g_ps.type);
        } break;
        case 'h': case 'H': {
            g_showHelp = !g_showHelp;
        } break;
        case 'p': case 'P': {
            g_paused = !g_paused;
        } break;
        case '+': {
            g_ps.emitRate = std::min(g_ps.emitRate + 50.f, 2000.f);
        } break;
        case '-': {
            g_ps.emitRate = std::max(g_ps.emitRate - 50.f, 10.f);
        } break;
        case 27: {
            exit(0);
        } break;
    }
}

void specialKey(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT: {
            g_ps.camYaw -= 5.f;
        } break;
        case GLUT_KEY_RIGHT: {
            g_ps.camYaw += 5.f;
        } break;
        case GLUT_KEY_UP: {
            g_ps.camPitch = std::min(g_ps.camPitch + 3.f, 89.f);
        } break;
        case GLUT_KEY_DOWN: {
            g_ps.camPitch = std::max(g_ps.camPitch - 3.f, -89.f);
        } break;
    }
}

void mouseButton(int btn, int state, int x, int y) {
    if (btn == GLUT_LEFT_BUTTON) {
        g_mouseDown = (state == GLUT_DOWN);
        g_mouseX = x;
        g_mouseY = y;
    }
    if (btn == 3) {
        g_ps.camDist = std::max(g_ps.camDist - 0.8f, 3.f);
    }
    if (btn == 4) {
        g_ps.camDist = std::min(g_ps.camDist + 0.8f, 40.f);
    }
}

void mouseMotion(int x, int y) {
    if (g_mouseDown) {
        int dx = x - g_mouseX, dy = y - g_mouseY;
        g_ps.camYaw += dx * 0.4f;
        g_ps.camPitch += dy * 0.4f;
        g_ps.camPitch = std::max(-89.f, std::min(89.f, g_ps.camPitch));
        g_mouseX = x;
        g_mouseY = y;
    }
}

void display() {
    DWORD now = GetTickCount();
    float dt = (now - g_lastTick) / 1000.f;
    if (dt > 0.1f) {
        dt = 0.1f;
    }
    g_lastTick = now;

    if (!g_paused) {
        g_ps.update(dt);
    }

    glClearColor(g_bgBrightness, g_bgBrightness, g_bgBrightness * 1.5f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)g_width / g_height, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float yawR = g_ps.camYaw * (float)M_PI / 180.f;
    float pitchR = g_ps.camPitch * (float)M_PI / 180.f;
    float cx = g_ps.camDist * cosf(pitchR) * sinf(yawR);
    float cy = g_ps.camDist * sinf(pitchR);
    float cz = g_ps.camDist * cosf(pitchR) * cosf(yawR);
    gluLookAt(cx, cy, cz, 0, 2, 0, 0, 1, 0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    drawGrid();

    glEnable(GL_TEXTURE_2D);
    g_ps.render();
    glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_width, 0, g_height);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);

    std::ostringstream ss;
    ss << "Effect: [" << emitterNames[g_ps.type] << "]"
       << "  Particles: " << g_ps.aliveCount
       << "  Time: " << (int)g_ps.totalTime << "s"
       << (g_paused ? "  [PAUSED]" : "");
    drawText2D(10, g_height - 20, ss.str(), 1.f, 0.9f, 0.3f);

    if (g_showHelp) {
        float lh = 18.f;
        float bx = 10.f, by = (float)(g_height - 50);
        const char* lines[] = {
            "--- Controls ---",
            "1-5  : Switch effect",
            "R    : Restart effect",
            "P    : Pause/Resume",
            "+/-  : Emit rate",
            "Arrow: Rotate camera",
            "Drag : Rotate camera",
            "Scroll: Zoom",
            "H    : Toggle help",
            "ESC  : Quit"
        };
        for (auto& l : lines) {
            drawText2D(bx, by, l, 0.7f, 0.9f, 1.f);
            by -= lh;
        }
    } else {
        drawText2D(10, (float)(g_height - 50), "H: show help", 0.5f, 0.7f, 0.9f);
    }

    {
        std::string ename = emitterNames[g_ps.type];
        drawText2D((float)(g_width / 2) - ename.size() * 5.f, 20, ename, 1.f, 0.6f, 0.2f);
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(g_width, g_height);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("3D Particle Engine  --  Press H for help");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    createParticleTexture();
    g_ps.init();
    g_ps.switchEmitter(EMITTER_FIRE);
    g_lastTick = GetTickCount();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKey);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
