/*
Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 3 - Robot Hunter

Added:
- Gunfire, Bullet, and Aim
- Per-robot hit effect and improved camera logic
- Bounding Sphere Collision Detection, Kill Count, Collider Toggle
- Restored wireframe/solid toggles
- Enlarged bright-green head visible from ESV
- Scoring, Timer, Mission Complete/Fail
- Fullscreen Toggle (F1)
- Popup Menu (Shift+Right-Click)
- Arcball Camera (ESV only): Left-drag rotate, Right-drag zoom
- Enemy Robot Motion (toggle 'm')
- Cleaned HUD (no instructions, printed to terminal)
*/

#include "helper.hpp"
#include <cmath>
#include <ctime>
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// ===== Window Globals =====
int WIN_W = 800;
int WIN_H = 600;
int ROBOT_COUNT = 10;

#define PI 3.14159265f

// ===== Cameras =====
Camera camFPV = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f };
Camera camESV = { 0.0f, 20.0f, 0.01f, 0.0f, 0.0f, 0.0f };
bool mainIsESV = false;
float cameraMS = 0.1f;

// ===== Arcball (ESV) =====
float arcballTheta = 0.0f;
float arcballPhi = 45.0f;
float arcballRadius = 20.0f;
int lastMouseX = 0, lastMouseY = 0;
bool leftDragging = false, rightDragging = false;

// ===== Game State =====
bool dancing = false;
int danceTimerDelay = 50;
bool groupDance = true;
int danceAngle = 0;
float torsoBounce = 0.05f * sinf(danceAngle * PI / 180.0f);

bool axies = false;
State state = SOLID;
bool clear = false;
bool isFullscreen = false;

Vector3 torsoColor = getColor(BLUE);
Vector3 headColor = getColor(YELLOW);
Vector3 armColor = getColor(GREEN);
Vector3 legColor = getColor(RED);

// ===== Robot Data =====
std::vector<float> robotSpeeds;
std::vector<float> robotOffsets;
std::vector<int> robotTypes;
std::vector<Vector3> robotPositions;
std::vector<float> robotHitTimers;
std::vector<bool> robotAlive;
bool confettiActive = false;

// ===== Bounding Spheres =====
std::vector<float> robotRadii;
bool showColliders = false;

// ===== Scoring / Mission =====
int killCount = 0;
int score = 0;
bool gameOver = false;
int startTime = 0;
std::string missionMsg = "";

// ===== Bullets =====
struct Bullet {
    Vector3 pos;
    Vector3 dir;
    float speed;
    bool active;
    bool hit;
};
std::vector<Bullet> bullets;
int bulletMode = 0;
float bulletSpeeds[3] = { 0.05f, 0.15f, 0.3f };
std::string bulletLabels[3] = { "slow", "fast", "very fast" };

// ===== Function Prototypes =====
void updateArcballCamera();
void toggleFullscreen();
void resetGame();
void menuHandler(int option);
void onMenuStatus(int status, int x, int y);

// ===== Utility Draws =====
void drawAxes() {
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
    glEnd();
}

void drawGroundPlane() {
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-100, -1.5f, -100);
    glVertex3f(-100, -1.5f, 100);
    glVertex3f(100, -1.5f, 100);
    glVertex3f(100, -1.5f, -100);
    glEnd();
}

// ===== Robots =====
void drawRobot(float angle, int type, int i);
void drawRobotAtIndex(int i);

// ===== Gun / Aim =====
void drawGunAndAim(bool isESVView) {
    // Player marker in ESV
    if (isESVView) {
        glPushMatrix();
        glTranslatef(camFPV.x, 1.5f, camFPV.z);
        glColor3f(0.0f, 1.0f, 0.0f);
        glutSolidCube(0.8);
        glPopMatrix();
    }

    // Aim sphere in FPV
    if (!isESVView) {
        glPushMatrix();
        glTranslatef(camFPV.x + camFPV.lx * 5.0f, camFPV.y, camFPV.z + camFPV.lz * 5.0f);
        glColor3f(1, 1, 1);
        glutSolidSphere(0.05, 10, 10);
        glPopMatrix();
    }
}

void drawBullets() {
    glColor3f(1, 1, 0);
    for (auto& b : bullets) {
        if (!b.active) continue;
        glPushMatrix();
        glTranslatef(b.pos.x, b.pos.y, b.pos.z);
        glutSolidSphere(0.1, 10, 10);
        glPopMatrix();
    }
}

void drawColliders() {
    if (!showColliders) return;
    glColor3f(1, 0, 1);
    for (int i = 0; i < ROBOT_COUNT; i++) {
        if (!robotAlive[i]) continue;
        glPushMatrix();
        glTranslatef(robotPositions[i].x, 0.0f, robotPositions[i].z);
        glutWireSphere(robotRadii[i], 10, 10);
        glPopMatrix();
    }
}

// ===== Bullet & Collision =====
void updateBullets() {
    for (auto& b : bullets) {
        if (!b.active) continue;

        b.pos.x += b.dir.x * b.speed;
        b.pos.y += b.dir.y * b.speed;
        b.pos.z += b.dir.z * b.speed;

        if (fabs(b.pos.x) > 100 || fabs(b.pos.z) > 100) {
            b.active = false;
            if (!b.hit) score -= 2;
            continue;
        }

        for (int i = 0; i < ROBOT_COUNT; i++) {
            if (!robotAlive[i]) continue;

            float dx = b.pos.x - robotPositions[i].x;
            float dz = b.pos.z - robotPositions[i].z;
            float dist = sqrtf(dx * dx + dz * dz);
            float bulletRadius = 0.1f;
            float combined = bulletRadius + robotRadii[i];

            if (dist < combined) {
                robotHitTimers[i] = 1.0f;
                robotAlive[i] = false;
                b.active = false;
                b.hit = true;
                killCount++;
                score += 10;
                PlaySound(TEXT("SystemAsterisk"), NULL, SND_ALIAS | SND_ASYNC);
                break;
            }
        }
    }

    for (int i = 0; i < ROBOT_COUNT; i++) {
        if (robotHitTimers[i] > 0.0f) {
            robotHitTimers[i] -= 0.016f;
            if (robotHitTimers[i] < 0.0f) robotHitTimers[i] = 0.0f;
        }
    }

    if (!gameOver && (killCount == ROBOT_COUNT)) {
        gameOver = true;
        missionMsg = "Mission Complete!";
    }
}

// ===== Arcball =====
void updateArcballCamera() {
    float t = arcballTheta * (PI / 180.0f);
    float p = arcballPhi * (PI / 180.0f);
    camESV.x = arcballRadius * sinf(t) * cosf(p);
    camESV.y = arcballRadius * sinf(p);
    camESV.z = arcballRadius * cosf(t) * cosf(p);
}

// ===== Cameras =====
void lookFromCamera(const Camera& cam, int viewW, int viewH) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)viewW / (double)viewH, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (cam.y > 5.0f)
        gluLookAt(cam.x, cam.y, cam.z, 0, 0, 0, 0, 1, 0);
    else
        gluLookAt(cam.x, cam.y, cam.z, cam.x + cam.lx, cam.y, cam.z + cam.lz, 0, 1, 0);
}

// ===== Robots =====
void drawRobot(float localAngle, int type, int i) {
    if (!robotAlive[i]) return;

    float torsoBounceLocal = 0.05f * sinf(localAngle * PI / 180.0f);
    float swing = sinf(localAngle * PI / 180.0f) * 30.0f;
    float kick = sinf(localAngle * PI / 180.0f) * 40.0f;
    float leftArmXrot = 0.0f, rightArmXrot = 0.0f;
    float leftLegXrot = 0.0f, rightLegXrot = 0.0f;

    if (type == 0) { leftArmXrot = swing; rightArmXrot = -swing; }
    else { leftLegXrot = kick; rightLegXrot = -kick; }

    Vector3 bodyCol = torsoColor;
    Vector3 legCol = legColor;
    if (robotHitTimers[i] > 0.0f) { bodyCol = Vector3(1, 0, 0); legCol = Vector3(1, 0, 0); }

    glPushMatrix();
    createObject(CUBE, Vector3(0, torsoBounceLocal, 0), Vector3(0, 0, 0), Vector3(0.8f, 1.0f, 0.4f), bodyCol);
    createObject(CUBE, Vector3(0, 0.8f, 0), Vector3(0, 0, 0), Vector3(0.6f, 0.6f, 0.6f), headColor);
    createObject(CUBE, Vector3(-0.55f, 0, 0), Vector3(leftArmXrot, 0, 0), Vector3(0.3f, 1.0f, 0.3f), armColor);
    createObject(CUBE, Vector3(0.55f, 0, 0), Vector3(-leftArmXrot, 0, 0), Vector3(0.3f, 1.0f, 0.3f), armColor);
    createObject(CUBE, Vector3(-0.2f, -1.0f, 0), Vector3(leftLegXrot, 0, 0), Vector3(0.3f, 1.0f, 0.3f), legCol);
    createObject(CUBE, Vector3(0.2f, -1.0f, 0), Vector3(rightLegXrot, 0, 0), Vector3(0.3f, 1.0f, 0.3f), legCol);
    glPopMatrix();
}

void drawRobotAtIndex(int i) {
    if (!robotAlive[i]) return;
    glPushMatrix();
    glTranslatef(robotPositions[i].x, 0, robotPositions[i].z);
    float localAngle = groupDance ? danceAngle : danceAngle * robotSpeeds[i] + robotOffsets[i];
    drawRobot(localAngle, 0, i);
    glPopMatrix();
}

void render3DSceneCommon() {
    drawGroundPlane();
    for (int i = 0; i < ROBOT_COUNT; ++i) drawRobotAtIndex(i);
    drawBullets();
    drawColliders();
    if (axies) drawAxes();
}

// ===== HUD =====
void drawHUDViewport() {
    int hudH = WIN_H / 8;
    int hudY = WIN_H - hudH;
    clearViewportArea(0, hudY, WIN_W, hudH);
    glViewport(0, hudY, WIN_W, hudH);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, hudH);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3f(1, 1, 1);

    static int elapsedAtEnd = 0;
    int elapsed = (int)(time(NULL) - startTime);
    if (gameOver) elapsed = elapsedAtEnd; else elapsedAtEnd = elapsed;
    int remaining = 30 - elapsed; if (remaining < 0) remaining = 0;

    float step = (float)hudH / 6;
    float y = hudH - step;
    void* font = (hudH > 100) ? GLUT_BITMAP_HELVETICA_18 : GLUT_BITMAP_HELVETICA_12;

    drawBitmapString(10, y, "Robot Hunter", font);
    y -= step;
    drawBitmapString(10, y, "Bullet Speed: " + bulletLabels[bulletMode], font);
    y -= step;
    drawBitmapString(10, y, "Score: " + std::to_string(score) + " | Time Left: " + std::to_string(remaining) + "s", font);
    y -= step;
    drawBitmapString(10, y, "Robots Killed: " + std::to_string(killCount) + " / " + std::to_string(ROBOT_COUNT), font);
    y -= step;
    drawBitmapString(10, y, "Scoring: +10 hit, -2 miss | 30s time limit", font);

    if (gameOver) drawBitmapString(WIN_W / 2 - 60, hudH / 2, missionMsg, font);
    glEnable(GL_DEPTH_TEST);
}

// ===== Viewports =====
void drawMainViewport() {
    int hudH = WIN_H / 8;
    int mainH = WIN_H - hudH;
    clearViewportArea(0, 0, WIN_W, mainH);
    glViewport(0, 0, WIN_W, mainH);

    if (mainIsESV) {
        updateArcballCamera();
        lookFromCamera(camESV, WIN_W, mainH);
    }
    else lookFromCamera(camFPV, WIN_W, mainH);

    render3DSceneCommon();
    drawGunAndAim(mainIsESV);

    // Mini view
    int miniW = WIN_W / 4, miniH = mainH / 4;
    int miniX = WIN_W - miniW, miniY = mainH - miniH;
    clearViewportArea(miniX, miniY, miniW, miniH);
    glViewport(miniX, miniY, miniW, miniH);

    if (mainIsESV) lookFromCamera(camFPV, miniW, miniH);
    else lookFromCamera(camESV, miniW, miniH);
    render3DSceneCommon();
    drawGunAndAim(!mainIsESV);
}

// ===== Display =====
void MyDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (clear) { glutSwapBuffers(); return; }

    if (!gameOver && (time(NULL) - startTime >= 30)) {
        gameOver = true;
        missionMsg = (killCount == ROBOT_COUNT) ? "Mission Complete!" : "Mission Fail!";
    }

    if (!gameOver) updateBullets();
    drawHUDViewport();
    drawMainViewport();
    glutSwapBuffers();
}

void reshape(int w, int h) { WIN_W = w; WIN_H = (h > 0 ? h : 1); glutPostRedisplay(); }

// ===== Keyboard =====
void keyboardInput(unsigned char key, int, int) {
    if (gameOver) return;

    switch (key) {
    case 'w': state = WIRE; glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
    case 's': state = SOLID; glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
    case 'c': showColliders = !showColliders; break;
    case 'a': axies = !axies; break;
    case 'b': bulletMode = (bulletMode + 1) % 3; break;
    case 'm': dancing = !dancing; if (dancing) glutTimerFunc(danceTimerDelay, danceTimer, 0); break;

    case ' ': {
        Bullet b;
        b.pos = Vector3(camFPV.x, camFPV.y, camFPV.z);
        b.dir = Vector3(camFPV.lx, 0.0f, camFPV.lz);
        b.speed = bulletSpeeds[bulletMode];
        b.active = true;
        b.hit = false;
        bullets.push_back(b);
    } break;

    case 27: exit(0); break;
    }
    glutPostRedisplay();
}

// ===== Special Keys =====
void specialKeys(int key, int, int) {
    if (gameOver) return;
    switch (key) {
    case GLUT_KEY_LEFT:  camFPV.angle -= 0.05f; break;
    case GLUT_KEY_RIGHT: camFPV.angle += 0.05f; break;
    case GLUT_KEY_UP:
        camFPV.x += sinf(camFPV.angle) * cameraMS;
        camFPV.z -= cosf(camFPV.angle) * cameraMS;
        break;
    case GLUT_KEY_DOWN:
        camFPV.x -= sinf(camFPV.angle) * cameraMS;
        camFPV.z += cosf(camFPV.angle) * cameraMS;
        break;
    case GLUT_KEY_F1: toggleFullscreen(); break;
    case GLUT_KEY_F2: mainIsESV = !mainIsESV; break;
    }
    camFPV.lx = sinf(camFPV.angle);
    camFPV.lz = -cosf(camFPV.angle);
    glutPostRedisplay();
}

// ===== Mouse =====
void mouseButton(int button, int state, int x, int y) {
    // Shift + Right-Click -> Popup menu
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        int mods = glutGetModifiers();
        if (mods & GLUT_ACTIVE_SHIFT) {
            glutAttachMenu(GLUT_RIGHT_BUTTON);
        }
        else {
            // ensure plain right-click does NOT open the menu
#ifdef FREEGLUT
            glutDetachMenu(GLUT_RIGHT_BUTTON);
#endif
        }
    }

    // Arcball only when ESV is main view
    if (!mainIsESV) return;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) { leftDragging = true; lastMouseX = x; lastMouseY = y; }
        else leftDragging = false;
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        if (state == GLUT_DOWN) { rightDragging = true; lastMouseX = x; lastMouseY = y; }
        else rightDragging = false;
    }
}

void mouseMotion(int x, int y) {
    if (!mainIsESV) return;
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (leftDragging) {
        arcballTheta += dx * 0.5f;
        arcballPhi += dy * 0.5f;
        if (arcballPhi > 89)  arcballPhi = 89;
        if (arcballPhi < -89) arcballPhi = -89;
    }
    if (rightDragging) {
        arcballRadius += dy * 0.1f;
        if (arcballRadius < 2.0f)   arcballRadius = 2.0f;
        if (arcballRadius > 100.0f) arcballRadius = 100.0f;
    }

    lastMouseX = x; lastMouseY = y;
    updateArcballCamera();
    glutPostRedisplay();
}

// When the popup menu closes, detach so Shift remains required
void onMenuStatus(int status, int, int) {
#ifdef FREEGLUT
    if (status == GLUT_MENU_NOT_IN_USE) {
        glutDetachMenu(GLUT_RIGHT_BUTTON);
    }
#endif
}

// ===== Fullscreen, Menu, Reset =====
void toggleFullscreen() {
    if (!isFullscreen) { glutFullScreen(); isFullscreen = true; }
    else { glutReshapeWindow(800, 600); isFullscreen = false; }
}

void resetGame() {
    killCount = 0; score = 0; bullets.clear();
    robotHitTimers.assign(ROBOT_COUNT, 0.0f);
    robotAlive.assign(ROBOT_COUNT, true);
    gameOver = false;
    missionMsg.clear();
    startTime = (int)time(NULL);
    arcballTheta = 0.0f; arcballPhi = 45.0f; arcballRadius = 20.0f;
    updateArcballCamera();
    glutPostRedisplay();
}

void menuHandler(int option) {
    if (option == 0) resetGame();
    else if (option == 1) exit(0);
}

// ===== Main =====
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Robot Hunter - Final");

    initRobots();
    robotHitTimers.resize(ROBOT_COUNT, 0.0f);
    robotAlive.resize(ROBOT_COUNT, true);
    robotRadii.resize(ROBOT_COUNT, 1.0f);
    startTime = (int)time(NULL);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    glutDisplayFunc(MyDisplay);
    glutIdleFunc(MyDisplay);
    glutKeyboardFunc(keyboardInput);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    // Build popup menu (detached by default; Shift+Right attaches temporarily)
    glutCreateMenu(menuHandler);
    glutAddMenuEntry("RESUME", 0);
    glutAddMenuEntry("EXIT", 1);
    glutMenuStatusFunc(onMenuStatus); // re-detach after use (FreeGLUT)

    updateArcballCamera();
    printInstructions(); // print to terminal
    glutMainLoop();
    return 0;
}
