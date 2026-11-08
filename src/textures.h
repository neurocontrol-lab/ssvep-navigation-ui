#pragma once

#include <GL/glut.h> // Required for GLuint, GLfloat

// Wall rendering utilities
void wall_vertical(GLfloat x, GLfloat y, GLfloat z, GLuint texture = 1); 
void wall_horizontal(GLfloat x, GLfloat y, GLfloat z, GLuint texture = 1); 

// Environment and Maze rendering
void print_maze(GLuint* walls); 
void sky(GLuint haze, GLuint upTop, GLuint downLo);
void floor(GLuint grnd);
