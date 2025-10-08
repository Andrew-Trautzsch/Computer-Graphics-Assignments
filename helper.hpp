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
extern bool musicPlaying;

extern Vector3 torsoColor;
extern Vector3 headColor;
extern Vector3 armColor;
extern Vector3 legColor;

Vector3 getColor(Color c);
const char* getColorName(Color c);
void printInstructions();
void createMenus();

void rotateX(int input);
void rotateY(int input);
void rotateZ(int input);

void setTorsoColor(int c);
void setHeadColor(int c);
void setArmColor(int c);
void setLegColor(int c);

void danceTimer(int value);

void menuProjection(int option);

//
//////////// BONUS
//
// helper function prototypes
void playHello();
void playDance();
void playBye();
//

#endif