/*
This file consist of global variables, helpful datastructures, and helper functions for main.cpp
*/

#ifndef HELPER_HPP
#define HELPER_HPP

#include <GL/glut.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <mmsystem.h> // having only <windows.h> didnt include the playSound() function for me, so i needed to directed include this lib, which is a sub lib of windows.h


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
	float lx, lz;  // Look direction, only need x and z
	float angle;   // Rotation angle in 
};

extern bool dancing;
extern int danceAngle;
extern float torsoBounce;

extern bool axies;
extern State state;
extern bool clear;
extern bool BnW;
extern Vector3 globalRot;
extern float zoom;
extern bool ortho;

extern Vector3 torsoColor;
extern Vector3 headColor;
extern Vector3 armColor;
extern Vector3 legColor;

extern float robotSpeeds[5];
extern float robotOffsets[5];
extern int robotTypes[5];
extern Vector3 robotPositions[5];

Vector3 getColor(Color c);
const char* getColorName(Color c);
void printInstructions();

void danceTimer(int value);

void createObject(Shape type, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color);

void clearViewportArea(int x, int y, int w, int h);
void initRobots();

#endif