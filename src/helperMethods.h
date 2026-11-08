#pragma once

#include <string>     // Required for std::string
#include <functional> // Required for std::function
#include <GL/glut.h>  // Required for GLuint, GLint, GLenum
#include <ctime>   // For std::clock() and CLOCKS_PER_SEC


GLint windowwidth();
GLint windowheight();

// Game State Utility
std::string timeLeft(const std::function<void(int, int, int, int)>& func, clock_t endTime);

// Texture Loader Utility
GLuint maketex(const char* tfile, GLint xSize, GLint ySize);

// Requires a current OpenGL context. Refresh rate is OS-reported, not measured.
struct DisplayTiming {
    bool vsyncEnabled;
    double refreshHz;
};
DisplayTiming initializeDisplayTiming();
