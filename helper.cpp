/*
This file contains "helper" functions that assist main functions in main.cpp
The printInstructions() and small utilities live here to reduce space in main.cpp
*/

#include "helper.hpp"
#include <ctime>

// used for color menu
Vector3 getColor(Color c) {
    switch (c) {
    case RED:    return Vector3(1, 0, 0);
    case GREEN:  return Vector3(0, 1, 0);
    case BLUE:   return Vector3(0, 0, 1);
    case YELLOW: return Vector3(1, 1, 0);
    case CYAN:   return Vector3(0, 1, 1);
    case MAGENTA:return Vector3(1, 0, 1);
    case WHITE:  return Vector3(1, 1, 1);
    default:     return Vector3(1, 1, 1);
    }
}

// used for color menu
const char* getColorName(Color c) {
    switch (c) {
    case RED:     return "Red";
    case GREEN:   return "Green";
    case BLUE:    return "Blue";
    case YELLOW:  return "Yellow";
    case CYAN:    return "Cyan";
    case MAGENTA: return "Magenta";
    case WHITE:   return "White";
    default:      return "Unknown";
    }
}

// Simple bitmap string (window-space for orthographic HUD)
void drawBitmapString(float x, float y, const std::string& text, void* font) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(font ? font : GLUT_BITMAP_HELVETICA_18, c);
    }
}

// instructions on using program
void printInstructions() {
    std::cout << "================= Assignment 3: Robot Hunter (Viewport + Robots) =================\n";
    std::cout << "Keyboard Controls:\n";
    std::cout << "  w : Display wireframe model of robots\n";
    std::cout << "  s : Display solid model of robots (default)\n";
    std::cout << "  p : Display vertex-only model of robots\n";
    std::cout << "  a : Toggle axes display (X=Red, Y=Green, Z=Blue)\n";
    std::cout << "  c : Toggle screen clearing (black background only)\n";
    std::cout << "  d : Toggle dancing animation on/off\n";
    std::cout << "  i : Toggle between group dancing and individual dancing\n";
    std::cout << "  q : Quit the program\n\n";

    std::cout << "Camera Controls (Arrow Keys):\n";
    std::cout << "  UP    : Move camera forward\n";
    std::cout << "  DOWN  : Move camera backward\n";
    std::cout << "  LEFT  : Rotate camera left (FPV)\n";
    std::cout << "  RIGHT : Rotate camera right (FPV)\n\n";

    std::cout << "Viewports:\n";
    std::cout << "  F2 : Toggle main view between FPV and ESV (bird's eye)\n";
    std::cout << "      Viewport1 (top): HUD 2D, height = window * 1/8\n";
    std::cout << "      Viewport2 (bottom): Main FPV/ESV, height = 7/8 window\n";
    std::cout << "      Viewport3: ESV mini in top-right corner of Viewport2\n";
    std::cout << "-------------------------------------------------------------------------------\n";
}

// Animation
extern bool dancing;
extern int danceAngle;
void danceTimer(int value) {
    if (dancing) {
        danceAngle = (danceAngle + 5) % 360;  // animate arms/legs rotation
        glutPostRedisplay();
        glutTimerFunc(50, danceTimer, 0);     // call again in ~50 ms
    }
}

// Draw primitive
extern State state;
void createObject(Shape type, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color)
{
    // If in wireframe mode, override color to white
    if (state == WIRE)
        color = Vector3(1.0f, 1.0f, 1.0f);

    glPushMatrix();

    // Apply transformations
    glColor3f(color.x, color.y, color.z);
    glTranslatef(position.x, position.y, position.z);
    glRotatef(rotation.x, 1, 0, 0);
    glRotatef(rotation.y, 0, 1, 0);
    glRotatef(rotation.z, 0, 0, 1);
    glScalef(scale.x, scale.y, scale.z);

    // Draw the correct object type
    if (type == CUBE)
    {
        switch (state) {
        case VERTEX:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(10.0f);  // creates points at all vertices
            glutSolidCube(1.0);  // solid gives real vertices
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case WIRE:
            glutWireCube(1.0);
            break;
        case SOLID:
            glutSolidCube(1.0);
            break;
        }
    }
    else if (type == SPHERE)
    {
        switch (state) {
        case VERTEX:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            glPointSize(5.0f);
            glutSolidSphere(0.5, 16, 16);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        case WIRE:
            glutWireSphere(0.5, 16, 16);
            break;
        case SOLID:
            glutSolidSphere(0.5, 16, 16);
            break;
        }
    }

    glPopMatrix(); // end object creation
}

// Used to prevent overlap of primary camera and corner cameras
void clearViewportArea(int x, int y, int w, int h) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
}

// Globals defined in main.cpp
extern std::vector<Vector3> robotPositions;
extern std::vector<float>   robotSpeeds;
extern std::vector<float>   robotOffsets;
extern std::vector<int>     robotTypes;
extern int ROBOT_COUNT;

// initializes robot randomness (position and dance properties)
void initRobots() {
    srand((unsigned int)time(NULL));
    robotPositions.resize(ROBOT_COUNT);
    robotSpeeds.resize(ROBOT_COUNT);
    robotOffsets.resize(ROBOT_COUNT);
    robotTypes.resize(ROBOT_COUNT);

    for (int i = 0; i < ROBOT_COUNT; i++) {
        // random positions in [-20,20)
        robotPositions[i] = Vector3((float)((rand() % 40) - 20), 0.0f, (float)((rand() % 40) - 20));

        // Randomize dance parameters:
        robotSpeeds[i] = 0.8f + (float)(rand() % 61) / 100.0f;   // 0.8 .. 1.41
        robotOffsets[i] = (float)(rand() % 360);                 // phase 0..359
        robotTypes[i] = rand() % 5;                            // 0..4 animation type
    }
}
