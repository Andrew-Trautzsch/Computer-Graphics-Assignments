/*
Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 3 - Robot Hunter

Implements:
- ROBOT_COUNT-driven enemy generation
- Three Viewports UI (HUD 1/8 orthographic, Main 7/8 perspective FPV/ESV, Mini ESV in upper-right)
- F2 toggles FPV vs ESV as the Main view
*/

#include "helper.hpp"
#include <cmath>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

int WIN_W = 800;
int WIN_H = 600;
int ROBOT_COUNT = 10; // <- change this to generate more/fewer robots

#define PI 3.14159265f

// Cameras
Camera camFPV = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f }; // First Person
Camera camESV = { 0.0f, 20.0f, 0.01f, 0.0f,  0.0f, 0.0f }; // Bird's Eye (look straight down)

// Camera control state
bool mainIsESV = false;    // toggled by F2; false = FPV main, true = ESV main

// Movement
float cameraMS = 0.1f;

// Animation / draw state (externs in helper.hpp)
bool dancing = false;
bool groupDance = true;
int  danceAngle = 0;
float torsoBounce = 0.05f * sinf(danceAngle * PI / 180.0f);

bool axies = false;
State state = SOLID;
bool clear = false;

Vector3 torsoColor = getColor(BLUE);
Vector3 headColor = getColor(YELLOW);
Vector3 armColor = getColor(GREEN);
Vector3 legColor = getColor(RED);

// Robot arrays now dynamic
std::vector<float>   robotSpeeds;
std::vector<float>   robotOffsets;
std::vector<int>     robotTypes;
std::vector<Vector3> robotPositions;

// BONUS Confetti (flag exposed in helper.hpp; hook your system back up later if you want)
bool confettiActive = false;

// ======= Drawing helpers =======
void drawAxes()
{
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); // X axis
    glVertex3f(0, 0, 0); glVertex3f(100, 0, 0);
    glColor3f(0, 1, 0); // Y axis
    glVertex3f(0, 0, 0); glVertex3f(0, 100, 0);
    glColor3f(0, 0, 1); // Z axis
    glVertex3f(0, 0, 0); glVertex3f(0, 0, 100);
    glEnd();
}

// Basic 200 x 200 dark-grey floor
void drawGroundPlane() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, -1.5f, -100.0f);
    glVertex3f(-100.0f, -1.5f, 100.0f);
    glVertex3f(100.0f, -1.5f, 100.0f);
    glVertex3f(100.0f, -1.5f, -100.0f);
    glEnd();
}

// Draw one robot given local dance parameters
void drawRobot(float localAngle, int type)
{
    float torsoBounceLocal = 0.05f * sinf(localAngle * PI / 180.0f);

    float headYrot = 0.0f;
    float leftArmXrot = 0.0f;
    float rightArmXrot = 0.0f;
    float bodyYaw = 0.0f;
    float lateralShift = 0.0f;
    float leftLegXrot = 0.0f;
    float rightLegXrot = 0.0f;

    float swing = (sinf(localAngle * PI / 180.0f) * 30.0f); // -30..30 deg
    float kick = (sinf(localAngle * PI / 180.0f) * 40.0f); // -40..40 deg
    // float bob   = (sinf(localAngle * PI / 180.0f) * 0.1f);  // reserved pattern

    switch (type) {
    case 0:
        leftArmXrot = swing;
        rightArmXrot = -swing;
        headYrot = (sinf(localAngle * PI / 180.0f * 0.5f) * 10.0f);
        leftLegXrot = -swing * 0.5f;
        rightLegXrot = swing * 0.5f;
        break;
    case 1:
        headYrot = (sinf(localAngle * PI / 180.0f * 2.0f) * 25.0f);
        leftArmXrot = swing * 0.3f;
        rightArmXrot = -swing * 0.3f;
        break;
    case 2:
        leftLegXrot = kick;
        rightLegXrot = -kick;
        leftArmXrot = kick * 0.25f;
        rightArmXrot = -kick * 0.25f;
        break;
    case 3:
        bodyYaw = fmodf(localAngle * 2.0f, 360.0f);
        leftArmXrot = 20.0f * sinf(localAngle * PI / 180.0f);
        rightArmXrot = -20.0f * sinf(localAngle * PI / 180.0f);
        break;
    case 4:
        // subtle sway
        lateralShift = 0.05f * sinf(localAngle * PI / 180.0f);
        leftArmXrot = swing * 0.4f;
        rightArmXrot = -swing * 0.4f;
        break;
    default:
        leftArmXrot = swing;
        rightArmXrot = -swing;
        break;
    }

    glPushMatrix();

    if (lateralShift != 0.0f) glTranslatef(lateralShift, 0.0f, 0.0f);

    // Torso (blue)
    createObject(CUBE,
        Vector3(0.0f, torsoBounceLocal, 0.0f),
        Vector3(0.0f, bodyYaw, 0.0f),
        Vector3(0.8f, 1.0f, 0.4f),
        torsoColor);

    // Head (yellow)
    createObject(CUBE,
        Vector3(0.0f, 0.8f, 0.0f),
        Vector3(0.0f, headYrot, 0.0f),
        Vector3(0.6f, 0.6f, 0.6f),
        headColor);

    // Left Arm (green)
    createObject(CUBE,
        Vector3(-0.55f, 0.0f, 0.0f),
        Vector3(leftArmXrot, 0.0f, 0.0f),
        Vector3(0.3f, 1.0f, 0.3f),
        armColor);

    // Right Arm (green)
    createObject(CUBE,
        Vector3(0.55f, 0.0f, 0.0f),
        Vector3(rightArmXrot, 0.0f, 0.0f),
        Vector3(0.3f, 1.0f, 0.3f),
        armColor);

    // Left Leg (red)
    createObject(CUBE,
        Vector3(-0.2f, -1.0f, 0.0f),
        Vector3(leftLegXrot, 0.0f, 0.0f),
        Vector3(0.3f, 1.0f, 0.3f),
        legColor);

    // Right Leg (red)
    createObject(CUBE,
        Vector3(0.2f, -1.0f, 0.0f),
        Vector3(rightLegXrot, 0.0f, 0.0f),
        Vector3(0.3f, 1.0f, 0.3f),
        legColor);

    glPopMatrix();
}

void drawRobotAtIndex(int i) {
    glPushMatrix();
    glTranslatef(robotPositions[i].x, 0.0f, robotPositions[i].z);

    float localAngle;
    int danceType;

    if (groupDance) { // All robots default to first dance
        localAngle = (float)danceAngle;
        danceType = 0;
    }
    else { // each robot has a unique dance
        localAngle = (float)danceAngle * robotSpeeds[i] + robotOffsets[i];
        danceType = robotTypes[i];
    }

    drawRobot(localAngle, danceType);
    glPopMatrix();
}

// ======= Cameras =======
void lookFromCamera(const Camera& cam, int viewW, int viewH) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)viewW / (double)viewH, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Bird's eye special case: look straight down
    if (cam.y > 5.0f) {
        gluLookAt(cam.x, cam.y, cam.z,
            cam.x, 0.0f, cam.z,
            0.0f, 0.0f, -1.0f); // up vector for top-down
    }
    else {
        gluLookAt(cam.x, cam.y, cam.z,
            cam.x + cam.lx, cam.y, cam.z + cam.lz,
            0.0f, 1.0f, 0.0f);
    }
}

void render3DSceneCommon() {
    drawGroundPlane();
    for (int i = 0; i < ROBOT_COUNT; ++i) drawRobotAtIndex(i);
    if (axies) drawAxes();
}

// ======= Viewport layout =======
void drawHUDViewport() {
    // Top strip: orthographic 2D
    int hudH = WIN_H / 8;
    int hudY = WIN_H - hudH;

    clearViewportArea(0, hudY, WIN_W, hudH);
    glViewport(0, hudY, WIN_W, hudH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, (GLdouble)WIN_W, 0.0, (GLdouble)hudH);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    // Simple text HUD placeholders per spec (Title, Score, etc.)
    glColor3f(1, 1, 1);
    drawBitmapString(10.0f, (float)hudH - 24.0f, "Robot Hunter - FPV/ESV (F2 to toggle)");
    drawBitmapString(10.0f, (float)hudH - 48.0f,
        std::string("Score: 000 | Robots Killed: 0 / ") + std::to_string(ROBOT_COUNT) +
        " | Time: 30s | Bullet: slow");

    glEnable(GL_DEPTH_TEST);
}

void drawMainViewport() {
    int hudH = WIN_H / 8;
    int mainH = WIN_H - hudH;
    int mainY = 0;

    clearViewportArea(0, mainY, WIN_W, mainH);
    glViewport(0, mainY, WIN_W, mainH);

    // Choose which camera is the main view
    if (mainIsESV) {
        lookFromCamera(camESV, WIN_W, mainH);
    }
    else {
        lookFromCamera(camFPV, WIN_W, mainH);
    }
    render3DSceneCommon();

    // Mini view in upper-right corner of the MAIN viewport
    int miniW = WIN_W / 4;
    int miniH = mainH / 4;
    int miniX = WIN_W - miniW;
    int miniY = mainH - miniH;

    clearViewportArea(miniX, miniY, miniW, miniH);
    glViewport(miniX, miniY, miniW, miniH);

    if (mainIsESV) {
        lookFromCamera(camFPV, miniW, miniH);
    }
    else {
        lookFromCamera(camESV, miniW, miniH);
    }
    render3DSceneCommon();
}

void MyDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (clear) { // quick toggle to blank screen
        glutSwapBuffers();
        return;
    }

    drawHUDViewport();
    drawMainViewport();

    glutSwapBuffers();
}

// Resize maintains 1:7 ratio naturally via hudH=H/8, mainH=7/8 H
void reshape(int w, int h) {
    if (h <= 0) h = 1;
    WIN_W = w;
    WIN_H = h;
    glutPostRedisplay();
}

// ======= Input =======
void specialKeys(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_LEFT: // rotate FPV camera left
        camFPV.angle -= 0.05f;
        camFPV.lx = sinf(camFPV.angle);
        camFPV.lz = -cosf(camFPV.angle);
        break;
    case GLUT_KEY_RIGHT: // rotate FPV camera right
        camFPV.angle += 0.05f;
        camFPV.lx = sinf(camFPV.angle);
        camFPV.lz = -cosf(camFPV.angle);
        break;
    case GLUT_KEY_UP: // move camera forward
        camFPV.x += camFPV.lx * cameraMS;
        camFPV.z += camFPV.lz * cameraMS;
        break;
    case GLUT_KEY_DOWN: // move camera backwards
        camFPV.x -= camFPV.lx * cameraMS;
        camFPV.z -= camFPV.lz * cameraMS;
        break;
    }
    glutPostRedisplay();
}

void functionKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F2:
        mainIsESV = !mainIsESV; // Toggle FPV/ESV
        break;
    default:
        break;
    }
}

void specialKeysCombined(int key, int x, int y) {
    if (key == GLUT_KEY_F2) {
        functionKeys(key, x, y);
    }
    else {
        specialKeys(key, x, y);
    }
    glutPostRedisplay();
}

void keyboardInput(unsigned char input, int x, int y)
{
    switch (input)
    {
    case 'a':
        axies = !axies;
        break;
    case 'c':
        clear = !clear;
        break;
    case 'd':
        dancing = !dancing;
        if (dancing) {
            glutTimerFunc(0, danceTimer, 0);
            PlaySound(TEXT("dance1.wav"), NULL, SND_FILENAME | SND_ASYNC);
        }
        else {
            PlaySound(NULL, 0, 0);
            confettiActive = false;
        }
        break;
    case 'i':
        groupDance = !groupDance;
        break;
    case 'p':
        state = VERTEX;
        break;
    case 'w':
        state = WIRE;
        break;
    case 's':
        state = SOLID;
        break;
    case 'q':
        exit(0);
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WIN_W, WIN_H);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Andrew Trautzsch, 811198871 - A3 Robot Hunter");

    initRobots(); // places ROBOT_COUNT robots

    // instruction helper functions
    printInstructions();

    // OpenGL setup
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);

    // Callbacks
    glutDisplayFunc(MyDisplay);
    glutKeyboardFunc(keyboardInput);
    glutSpecialFunc(specialKeysCombined);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
