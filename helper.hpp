/*
This file consists of global variables, helpful datastructures, and helper functions for main.cpp
*/
#ifndef HELPER_HPP
#define HELPER_HPP

#include <GL/glut.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

struct Vector3
{
    Vector3(float x_ = 0.0f, float y_ = 0.0f, float z_ = 0.0f) : x(x_), y(y_), z(z_) {}
    float x, y, z;
};

enum Shape
{
    CUBE,
    SPHERE
};

enum State
{
    SOLID,
    WIRE,
    VERTEX
};

enum Color {
    RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE
};

struct Camera {
    float x, y, z; // Position
    float lx, lz;  // Look direction (x,z) for FPV
    float angle;   // Rotation angle (radians) for FPV
};

struct Bullet {
    Vector3 pos;
    Vector3 dir;
    float speed;
    bool active;
    bool hit;
};

// Externs provided/owned by main.cpp
extern int WIN_W;
extern int WIN_H;
extern int ROBOT_COUNT;

extern bool dancing;
extern int danceAngle;
extern float torsoBounce;

extern bool axies;
extern State state;
extern bool clear;

extern Vector3 torsoColor;
extern Vector3 headColor;
extern Vector3 armColor;
extern Vector3 legColor;

// Robot state now dynamic to honor ROBOT_COUNT
extern std::vector<float>   robotSpeeds;
extern std::vector<float>   robotOffsets;
extern std::vector<int>     robotTypes;
extern std::vector<Vector3> robotPositions;

// Confetti control (used by helper.cpp render utilities if you re-enable your bonus)
extern bool confettiActive;

// Colors / text utils
Vector3 getColor(Color c);
const char* getColorName(Color c);

// Console instructions
void printInstructions();

// Animation timer
void danceTimer(int value);

// Primitive builder
void createObject(Shape type, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color);

// Viewport clearing utility
void clearViewportArea(int x, int y, int w, int h);

// Initialize robots (positions & dance params) using ROBOT_COUNT
void initRobots();

// Simple bitmap text utility for HUD
void drawBitmapString(float x, float y, const std::string& text, void* font = GLUT_BITMAP_HELVETICA_18);



#endif
