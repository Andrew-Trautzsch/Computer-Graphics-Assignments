/*
Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 4 - Robot Hunter
*/

#include "helper.hpp"

// ===== Texture Loading (stb_image) =====
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint loadTexture(const char* filename)
{
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        std::cout << "[ERROR] Failed to load: " << filename << "\n";
        std::cout << "reason: " << stbi_failure_reason() << "\n";
    }

    GLenum format = (channels == 3) ? GL_RGB : GL_RGBA;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
        0, format,
        width, height,
        0, format,
        GL_UNSIGNED_BYTE,
        data);

    stbi_image_free(data);
    return texID;
}

// ===== Window Globals =====
int WIN_W = 800;
int WIN_H = 600;

#define PI 3.14159265f

// ===== Cameras =====
Camera camFPV = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f };
Camera camESV = { 0.0f, 20.0f, 0.01f, 0.0f, 0.0f, 0.0f };
bool mainIsESV = false;
float cameraMS = 0.1f;
int lightingMode = 0;
int shadingMode = 1;

// ===== Arcball (ESV) =====
float arcballTheta = 0.0f;
float arcballPhi = 45.0f;
float arcballRadius = 20.0f;
int lastMouseX = 0, lastMouseY = 0;
bool leftDragging = false, rightDragging = false;

// ===== Game State =====
bool dancing = false;           
bool groupDance = true;         
int  danceAngle = 0;            
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
int ROBOT_COUNT = 10;
std::vector<float>   robotSpeeds;
std::vector<float>   robotOffsets;
std::vector<int>     robotTypes;
std::vector<Vector3> robotPositions;
std::vector<bool>    robotAlive;
std::vector<float>   robotJumpPhase;
std::vector<float>   robotYOffset;

// ===== Bounding Spheres =====
std::vector<float> robotRadi;
bool showColliders = false;

// ===== Scoring / Mission =====
int  killCount = 0;
int  score = 0;
bool gameOver = false;
int  startTime = 0;
std::string missionMsg = "";

// ===== Bullets =====
std::vector<Bullet> bullets;
int   bulletMode = 0;
float bulletSpeeds[3] = { 0.05f, 0.15f, 0.3f };
std::string bulletLabels[3] = { "slow", "fast", "very fast" };

std::vector<Vector3> robotDirs;

static float frand(float a, float b) {
    return a + (b - a) * (float(rand()) / float(RAND_MAX));
}
static Vector3 randDir2D() {
    float x = frand(-1.0f, 1.0f);
    float z = frand(-1.0f, 1.0f);
    float len = std::sqrt(x * x + z * z);
    if (len < 1e-3f) return Vector3(1, 0, 0);
    return Vector3(x / len, 0, z / len);
}

bool soundEnabled = true;

// Forward for BGM resume timer
void PlayBGMStart() {
    if (!soundEnabled) return;
    PlaySound(TEXT("bgm.wav"), NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}

void bgmResumeTimer(int) {
    if (!gameOver && soundEnabled) PlayBGMStart();
}

void PlayShoot() {
    if (!soundEnabled) return;
    PlaySound(TEXT("shoot.wav"), NULL, SND_ASYNC | SND_FILENAME);
    glutTimerFunc(700, bgmResumeTimer, 0);
}

void PlayHit() {
    if (!soundEnabled) return;
    PlaySound(TEXT("hit.wav"), NULL, SND_ASYNC | SND_FILENAME);
    glutTimerFunc(700, bgmResumeTimer, 0);
}

void StopAllSounds() {
    PlaySound(NULL, 0, 0);
}

// ===== Texture Globals =====
GLuint torsoTex = 0;
GLuint headTex = 0;
GLuint armTex = 0;
GLuint legTex = 0;
GLuint groundTex = 0;
GLuint skyTex = 0;

// ===== Popup Menu State (ESC) =====
bool showMenu = false;          
int  menuSelection = 0;         

struct MenuButton {
    float x, y, w, h;
};

MenuButton menuButtons[3];   

// ===== Function Prototypes =====
void updateArcballCamera();
void toggleFullscreen();
void resetGame();
void menuHandler(int option);
void onMenuStatus(int status, int x, int y);
void setupLighting();
void handleMenuSelection(int option);

void drawMenuOverlay() {
    // Use full window for the overlay
    glViewport(0, 0, WIN_W, WIN_H);

    // Setup 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, WIN_W, 0.0, WIN_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark translucent background
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f((float)WIN_W, 0.0f);
    glVertex2f((float)WIN_W, (float)WIN_H);
    glVertex2f(0.0f, (float)WIN_H);
    glEnd();

    // Menu title
    glColor3f(1.0f, 1.0f, 1.0f);
    drawBitmapString(WIN_W * 0.5f - 90.0f, WIN_H * 0.75f,
        "Robot Hunter - PAUSED",
        GLUT_BITMAP_HELVETICA_18);

    // Buttons
    const char* labels[3] = { "NEW GAME", "RESUME", "EXIT" };

    float boxW = 260.0f;
    float boxH = 40.0f;
    float centerX = WIN_W * 0.5f;
    float startY = WIN_H * 0.55f;

    for (int i = 0; i < 3; ++i) {
        float bx = centerX - boxW * 0.5f;
        float by = startY - i * (boxH + 15.0f);

        // store clickable regions for the mouse
        menuButtons[i].x = bx;
        menuButtons[i].y = by;
        menuButtons[i].w = boxW;
        menuButtons[i].h = boxH;

        if (i == menuSelection)
            glColor3f(0.8f, 0.8f, 0.2f); // highlighted
        else
            glColor3f(0.3f, 0.3f, 0.3f); // normal

        glBegin(GL_QUADS);
        glVertex2f(bx, by);
        glVertex2f(bx + boxW, by);
        glVertex2f(bx + boxW, by + boxH);
        glVertex2f(bx, by + boxH);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawBitmapString(bx + 20.0f, by + boxH * 0.3f,
            labels[i],
            GLUT_BITMAP_HELVETICA_18);
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void handleMenuSelection(int option) {
    switch (option) {
    case 0: // NEW GAME
        showMenu = false;
        resetGame();
        break;
    case 1: // RESUME
        showMenu = false;
        break;
    case 2: // EXIT
        StopAllSounds();
        exit(0);
        break;
    }
}


// ===== Utility Draws =====
void drawAxes() {
    glBegin(GL_LINES);
    glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(10, 0, 0);
    glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
    glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
    glEnd();
}

void drawSkybox(const Camera& cam) {
    if (skyTex == 0) return;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTex);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(cam.x, cam.y, cam.z);

    float S = 80.0f; // size of the sky cube

    glBegin(GL_QUADS);
    // +Z (front)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-S, -S, S);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(S, -S, S);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(S, S, S);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-S, S, S);

    // -Z (back)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(S, -S, -S);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-S, -S, -S);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-S, S, -S);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(S, S, -S);

    // +X (right)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(S, -S, S);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(S, -S, -S);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(S, S, -S);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(S, S, S);

    // -X (left)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-S, -S, -S);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-S, -S, S);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-S, S, S);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-S, S, -S);

    // +Y (top)
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-S, S, S);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(S, S, S);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(S, S, -S);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-S, S, -S);
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void drawGroundPlane()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTex);

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(0.0f, 0.0f);  glVertex3f(-100.0f, -1.5f, -100.0f);
    glTexCoord2f(8.0f, 0.0f);  glVertex3f(100.0f, -1.5f, -100.0f);
    glTexCoord2f(8.0f, 8.0f);  glVertex3f(100.0f, -1.5f, 100.0f);
    glTexCoord2f(0.0f, 8.0f);  glVertex3f(-100.0f, -1.5f, 100.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);
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

    // Robot colliders (magenta spheres)
    glColor3f(1, 0, 1);
    for (int i = 0; i < ROBOT_COUNT; i++) {
        if (!robotAlive[i]) continue;
        glPushMatrix();
        glTranslatef(robotPositions[i].x, robotYOffset[i], robotPositions[i].z);
        glutWireSphere(robotRadi[i], 10, 10);
        glPopMatrix();
    }

    // Obstacle colliders (cyan cubes)
    glColor3f(0, 1, 1);
    for (auto& o : obstacles) {
        if (!o.active) continue;

        glPushMatrix();
        glTranslatef(o.x, o.y + 1.0f, o.z);
        float size = 2.0f * o.scale;
        glutWireCube(size * 2.0f);
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

        // --- OBSTACLE BLOCKING ---
        for (auto& o : obstacles)
        {
            if (!o.active) continue;

            float size = 2.0f * o.scale;

            if (b.pos.x > o.x - size && b.pos.x < o.x + size &&
                b.pos.z > o.z - size && b.pos.z < o.z + size)
            {
                b.active = false;
                b.hit = true;
            }
        }

        for (int i = 0; i < ROBOT_COUNT; i++) {
            if (!robotAlive[i]) continue;

            float dx = b.pos.x - robotPositions[i].x;
            float dz = b.pos.z - robotPositions[i].z;
            float dist = sqrtf(dx * dx + dz * dz);
            float bulletRadius = 0.1f;
            float combined = bulletRadius + robotRadi[i];

            if (dist < combined) {
                robotAlive[i] = false;
                b.active = false;
                b.hit = true;
                killCount++;
                score += 10;

                PlayHit();
                break;
            }
        }
    }

    if (!gameOver && (killCount == ROBOT_COUNT)) {
        gameOver = true;
        missionMsg = "Mission Complete!";
    }
}

void updateRobotMotion() {
    if (gameOver) return;

    // Walk bounds (slightly inside ground so they don't step off visually)
    const float XZ_MIN = -95.0f;
    const float XZ_MAX = 95.0f;

    for (int i = 0; i < ROBOT_COUNT; ++i) {
        if (!robotAlive[i]) continue;

        // Jumping disrupt mode when 'm' is on
        if (dancing) {
            robotJumpPhase[i] += 0.08f + 0.12f * robotSpeeds[i]; // slower frequency
            float s = std::sin(robotJumpPhase[i] * 0.05f);
            robotYOffset[i] = (s > 0.0f ? s * 2.0f : 0.0f); // higher amplitude
        }
        else {
            robotYOffset[i] = 0.0f;

            float walkSpeed = 0.002f + 0.002f * robotSpeeds[i];

            float nextX = robotPositions[i].x + robotDirs[i].x * walkSpeed;
            float nextZ = robotPositions[i].z + robotDirs[i].z * walkSpeed;

            // Check shelves as blockers
            if (robotBlockedByObstacle(robotPositions[i].x, robotPositions[i].z, nextX, nextZ)) {
                // bounce off: flip direction, but don't move into the shelf
                robotDirs[i].x = -robotDirs[i].x;
                robotDirs[i].z = -robotDirs[i].z;
            }
            else {
                robotPositions[i].x = nextX;
                robotPositions[i].z = nextZ;
            }

            // Occasionally pick a new direction
            if ((rand() % 200) == 0) {
                robotDirs[i] = randDir2D();
            }


            // Bounce off bounds
            if (robotPositions[i].x < XZ_MIN || robotPositions[i].x > XZ_MAX) {
                robotDirs[i].x = -robotDirs[i].x;
                robotPositions[i].x = std::max(std::min(robotPositions[i].x, XZ_MAX), XZ_MIN);
            }
            if (robotPositions[i].z < XZ_MIN || robotPositions[i].z > XZ_MAX) {
                robotDirs[i].z = -robotDirs[i].z;
                robotPositions[i].z = std::max(std::min(robotPositions[i].z, XZ_MAX), XZ_MIN);
            }
        }
    }

    danceAngle = (danceAngle + 3) % 360;
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

    glPushMatrix();
    glTranslatef(0.0f, robotYOffset[i], 0.0f);

    // === TORSO (textured) ===
    glPushMatrix();
    glTranslatef(0.0f, torsoBounceLocal, 0.0f);
    glScalef(0.8f, 1.0f, 0.4f);
    drawTexturedCube(torsoTex);
    glPopMatrix();

    // === HEAD (textured) ===
    glPushMatrix();
    glTranslatef(0.0f, torsoBounceLocal + 0.8f, 0.0f);
    glScalef(0.6f, 0.6f, 0.6f);
    drawTexturedCube(headTex);
    glPopMatrix();

    // === LEFT ARM (textured) ===
    glPushMatrix();
    glTranslatef(-0.55f, torsoBounceLocal + 0.0f, 0.0f);
    glRotatef(leftArmXrot, 1, 0, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    drawTexturedCube(armTex);
    glPopMatrix();

    // === RIGHT ARM (textured) ===
    glPushMatrix();
    glTranslatef(0.55f, torsoBounceLocal + 0.0f, 0.0f);
    glRotatef(-leftArmXrot, 1, 0, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    drawTexturedCube(armTex);
    glPopMatrix();

    // === LEFT LEG (textured) ===
    glPushMatrix();
    glTranslatef(-0.2f, torsoBounceLocal - 1.0f, 0.0f);
    glRotatef(leftLegXrot, 1, 0, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    drawTexturedCube(legTex);
    glPopMatrix();

    // === RIGHT LEG (textured) ===
    glPushMatrix();
    glTranslatef(0.2f, torsoBounceLocal - 1.0f, 0.0f);
    glRotatef(rightLegXrot, 1, 0, 0);
    glScalef(0.3f, 1.0f, 0.3f);
    drawTexturedCube(legTex);
    glPopMatrix();

    glPopMatrix();
}

void drawRobotAtIndex(int i) {
    if (!robotAlive[i]) return;
    glPushMatrix();
    glTranslatef(robotPositions[i].x, 0.0f, robotPositions[i].z);
    float localAngle = groupDance ? danceAngle : danceAngle * robotSpeeds[i] + robotOffsets[i];
    drawRobot(localAngle, 0, i);
    glPopMatrix();
}

void render3DSceneCommon() {
    drawGroundPlane();
    drawObstacles();

    for (int i = 0; i < ROBOT_COUNT; ++i) drawRobotAtIndex(i);
    drawBullets();
    drawColliders();
    if (axies) drawAxes();
}


void drawFPVGun() {
    glPushMatrix();

    glTranslatef(camFPV.x + camFPV.lx * 0.7f,
        camFPV.y - 0.65f,
        camFPV.z + camFPV.lz * 0.7f);

    float yawDegrees = (-camFPV.angle) * (180.0f / PI) + 90.0f;
    glRotatef(yawDegrees, 0.0f, 1.0f, 0.0f);

    glDisable(GL_TEXTURE_2D);
    glColor3f(0.8f, 0.35f, 0.15f);

    // Handle
    glPushMatrix();
    glTranslatef(0.3f, -0.45f, 0.0f);
    glScalef(0.2f, 0.75f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Main body
    glPushMatrix();
    glTranslatef(0.55f, -0.12f, 0.0f);
    glScalef(0.8f, 0.4f, 0.4f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Long barrel
    glPushMatrix();
    glTranslatef(1.2f, -0.12f, 0.0f);
    glScalef(1.5f, 0.24f, 0.24f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.8f, -0.12f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    GLUquadric* q = gluNewQuadric();
    gluQuadricNormals(q, GLU_SMOOTH);
    gluCylinder(q, 0.12f, 0.12f, 1.0f, 20, 8);
    gluDeleteQuadric(q);
    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
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
    // used to keep text well fit on hud
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

    // ----- Main view -----
    bool mainIsFPV = !mainIsESV;

    if (mainIsESV) {
        updateArcballCamera();
        lookFromCamera(camESV, WIN_W, mainH);
        drawSkybox(camESV);
    }
    else {
        lookFromCamera(camFPV, WIN_W, mainH);
        drawSkybox(camESV);
    }

    setupLighting();
    glShadeModel(shadingMode == 0 ? GL_FLAT : GL_SMOOTH);

    render3DSceneCommon();

    if (mainIsFPV) {
        drawGunAndAim(false);
        drawFPVGun();
    }
    else {
        drawGunAndAim(true);
    }

    // ----- Mini view -----
    int miniW = WIN_W / 4, miniH = mainH / 4;
    int miniX = WIN_W - miniW, miniY = mainH - miniH;
    clearViewportArea(miniX, miniY, miniW, miniH);
    glViewport(miniX, miniY, miniW, miniH);

    bool miniIsFPV = !mainIsFPV;

    if (miniIsFPV) {
        lookFromCamera(camFPV, miniW, miniH);
        drawSkybox(camESV);
    }
    else {
        lookFromCamera(camESV, miniW, miniH);
        drawSkybox(camESV);
    }

    setupLighting();
    glShadeModel(shadingMode == 0 ? GL_FLAT : GL_SMOOTH);

    render3DSceneCommon();

    if (miniIsFPV) {
        drawGunAndAim(false);
        drawFPVGun();
    }
    else {
        drawGunAndAim(true);
    }
}



// ===== Display =====
void MyDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (clear) {
        glutSwapBuffers();
        return;
    }

    // Time limit only counts while not in menu
    if (!gameOver && !showMenu && (time(NULL) - startTime >= 30)) {
        gameOver = true;
        missionMsg = (killCount == ROBOT_COUNT) ? "Mission Complete!" : "Mission Fail!";
    }

    // Only update game when NOT paused by menu
    if (!gameOver && !showMenu) {
        updateRobotMotion();
        updateBullets();
    }

    drawHUDViewport();
    drawMainViewport();

    // Draw ESC menu overlay on top of everything
    if (showMenu) {
        drawMenuOverlay();
    }

    glutSwapBuffers();
}


void reshape(int w, int h) { WIN_W = w; WIN_H = (h > 0 ? h : 1); glutPostRedisplay(); }

// ===== Keyboard =====
void keyboardInput(unsigned char key, int, int) {
    // ESC always toggles the menu on/off
    if (key == 27) {   // ESC
        showMenu = !showMenu;
        glutPostRedisplay();
        return;
    }

    // If menu is open, only handle selection keys here
    if (showMenu) {
        if (key == 13 || key == ' ') {  // Enter or Space
            handleMenuSelection(menuSelection);
        }
        return;
    }

    // After mission end, ignore normal controls
    if (gameOver) return;

    switch (key) {
    case 'w': state = WIRE;  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
    case 's': state = SOLID; glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
    case 'c': showColliders = !showColliders; break;
    case 'a': axies = !axies; break;
    case 'b': bulletMode = (bulletMode + 1) % 3; break;
    case 'm':
        dancing = !dancing;
        // reset phases for crisp start/stop of jumping
        if (dancing) {
            for (int i = 0; i < ROBOT_COUNT; ++i) robotJumpPhase[i] = frand(0.0f, 100.0f);
        }
        else {
            for (int i = 0; i < ROBOT_COUNT; ++i) robotYOffset[i] = 0.0f;
        }
        break;
    case 'o':
    case 'O':
        showImportedModel = !showImportedModel;
        break;
    case 'l':
    case 'L':
        lightingMode = (lightingMode == 0 ? 1 : 0);
        printf("Lighting: %s\n", lightingMode == 0 ? "Directional" : "Point");
        break;


    case ' ': {
        Bullet b;
        b.pos = Vector3(camFPV.x, camFPV.y, camFPV.z);
        b.dir = Vector3(camFPV.lx, 0.0f, camFPV.lz);
        b.speed = bulletSpeeds[bulletMode];
        b.active = true;
        b.hit = false;
        bullets.push_back(b);

        PlayShoot();
    } break;

    case 27:
        StopAllSounds();
        exit(0);
        break;
    }
    glutPostRedisplay();
}

// ===== Special Keys =====
void specialKeys(int key, int, int) {
    // Menu navigation when ESC overlay is open
    if (showMenu) {
        if (key == GLUT_KEY_UP) {
            // move selection up (wrap around)
            menuSelection = (menuSelection + 2) % 3; // +2  -1 mod 3
        }
        else if (key == GLUT_KEY_DOWN) {
            // move selection down (wrap around)
            menuSelection = (menuSelection + 1) % 3;
        }
        glutPostRedisplay();
        return;
    }

    if (gameOver) return;

    switch (key) {
    case GLUT_KEY_LEFT:
        camFPV.angle -= 0.05f;
        break;
    case GLUT_KEY_RIGHT:
        camFPV.angle += 0.05f;
        break;
    case GLUT_KEY_UP:
        camFPV.x += sinf(camFPV.angle) * cameraMS;
        camFPV.z -= cosf(camFPV.angle) * cameraMS;
        break;
    case GLUT_KEY_DOWN:
        camFPV.x -= sinf(camFPV.angle) * cameraMS;
        camFPV.z += cosf(camFPV.angle) * cameraMS;
        break;
    case GLUT_KEY_F1:
        toggleFullscreen();
        break;
    case GLUT_KEY_F2:
        mainIsESV = !mainIsESV;
        break;
    case GLUT_KEY_F3:
        soundEnabled = !soundEnabled;
        if (!soundEnabled) {
            StopAllSounds();
        }
        else if (!gameOver) {
            PlayBGMStart();
        }
        break;
    case GLUT_KEY_F4:
        shadingMode = (shadingMode == 0 ? 1 : 0);
        break;
    }

    camFPV.lx = sinf(camFPV.angle);
    camFPV.lz = -cosf(camFPV.angle);
    glutPostRedisplay();
}


// ===== Mouse =====
void mouseButton(int button, int stateM, int x, int y) {
    if (showMenu) {
        if (button == GLUT_LEFT_BUTTON && stateM == GLUT_DOWN) {
            float mx = static_cast<float>(x);
            float my = static_cast<float>(WIN_H - y);

            for (int i = 0; i < 3; ++i) {
                MenuButton& b = menuButtons[i];
                if (mx >= b.x && mx <= b.x + b.w &&
                    my >= b.y && my <= b.y + b.h) {
                    handleMenuSelection(i);
                    break;
                }
            }
        }
        return; // don't let clicks control the game while menu is open
    }

    // Arcball only when ESV is main view
    if (!mainIsESV) return;

    if (button == GLUT_LEFT_BUTTON) {
        if (stateM == GLUT_DOWN) { leftDragging = true; lastMouseX = x; lastMouseY = y; }
        else leftDragging = false;
    }
    else if (button == GLUT_RIGHT_BUTTON) {
        if (stateM == GLUT_DOWN) { rightDragging = true; lastMouseX = x; lastMouseY = y; }
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
    if (status == GLUT_MENU_NOT_IN_USE) {
        glutDetachMenu(GLUT_RIGHT_BUTTON);
    }
}

// ===== Fullscreen, Menu, Reset =====
void toggleFullscreen() {
    if (!isFullscreen) { glutFullScreen(); isFullscreen = true; }
    else { glutReshapeWindow(800, 600); isFullscreen = false; }
}

void resetGame() {
    killCount = 0; score = 0; bullets.clear();
    robotAlive.assign(ROBOT_COUNT, true);
    gameOver = false;
    missionMsg.clear();
    startTime = (int)time(NULL);
    arcballTheta = 0.0f; arcballPhi = 45.0f; arcballRadius = 20.0f;

    for (int i = 0; i < ROBOT_COUNT; ++i) {
        robotDirs[i] = randDir2D();
        robotJumpPhase[i] = frand(0.0f, 100.0f);
        robotYOffset[i] = 0.0f;
    }
    PlayBGMStart();
    
    updateArcballCamera();
    glutPostRedisplay();
}

void menuHandler(int option) {
    if (option == 0) resetGame();
    else if (option == 1) { StopAllSounds(); exit(0); }
}

void setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);


    GLfloat globalAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    if (lightingMode == 0)
    {
        // Directional Light (Sun)
        glEnable(GL_LIGHT0);
        glDisable(GL_LIGHT1);

        GLfloat diffuse[] = { 1.2f, 1.2f, 1.2f, 1.0f };
        GLfloat ambient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
        GLfloat specular[] = { 1.1f, 1.1f, 1.1f, 1.0f };
        GLfloat direction[] = { -0.4f, -1.0f, -0.3f, 0.0f };

        glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT0, GL_POSITION, direction);
    }
    else
    {
        // Point Light (Bulb)
        glEnable(GL_LIGHT1);
        glDisable(GL_LIGHT0);

        GLfloat diffuse[] = { 1.5f, 1.5f, 1.5f, 1.0f };
        GLfloat ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
        GLfloat specular[] = { 1.2f, 1.2f, 1.2f, 1.0f };
        GLfloat position[] = { camFPV.x, camFPV.y + 3.0f, camFPV.z, 1.0f };

        glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
        glLightfv(GL_LIGHT1, GL_POSITION, position);

        // Adjust attenuation for wider reach
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.005f);
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.001f);
    }
}




// ===== Main =====
int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));

    glutInit(&argc, argv);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_NORMALIZE);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Andrew Trautzsch - 811198871");

    loadModel("models/utility_box_02_1k.fbx");
    generateObstacles(5);


    // Initialize robots
    initRobots();
    robotAlive.resize(ROBOT_COUNT, true);
    robotRadi.resize(ROBOT_COUNT, 1.0f);


    robotDirs.resize(ROBOT_COUNT);
    robotJumpPhase.resize(ROBOT_COUNT, 0.0f);
    robotYOffset.resize(ROBOT_COUNT, 0.0f);
    for (int i = 0; i < ROBOT_COUNT; ++i) {
        robotDirs[i] = randDir2D();
        robotJumpPhase[i] = frand(0.0f, 100.0f);
        robotYOffset[i] = 0.0f;
    }
    //

    startTime = (int)time(NULL);

    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

    // Load textures
    torsoTex = loadTexture("textures/torso.png");
    std::cout << "TorsoTex ID = " << torsoTex << "\n";

    headTex = loadTexture("textures/head.png");
    std::cout << "HeadTex ID = " << headTex << "\n";

    armTex = loadTexture("textures/arm.png");
    std::cout << "ArmTex ID = " << armTex << "\n";

    legTex = loadTexture("textures/leg.png");
    std::cout << "LegTex ID = " << legTex << "\n";

    groundTex = loadTexture("textures/ground.png");
    std::cout << "GroundTex ID = " << groundTex << "\n";

    skyTex = loadTexture("textures/sky.jpg");
    std::cout << "SkyTex ID = " << skyTex << "\n";

    glutDisplayFunc(MyDisplay);
    glutIdleFunc(MyDisplay);
    glutKeyboardFunc(keyboardInput);
    glutSpecialFunc(specialKeys);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutCreateMenu(menuHandler);
    glutAddMenuEntry("NEW GAME", 0);
    glutAddMenuEntry("EXIT", 1);
    glutMenuStatusFunc(onMenuStatus);

    updateArcballCamera();
    printInstructions();


    PlayBGMStart();


    glutMainLoop();
    return 0;
}
