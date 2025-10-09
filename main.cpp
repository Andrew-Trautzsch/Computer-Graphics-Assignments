/*
Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 2 - Moving Cameras and Humanoid Robots

BONUS:
Added a confetti particle system. Creates particles with pos and veloctiy above robots, which spreads and falls, once hitting y = -1, resets to a new posistion and repeats
Can be toggled while dancing with 'f', set max of 100 particles at a time

all sections that have bonus related code are marked with

//
//////////// BONUS
//
	(content)
//
*/

#include "helper.hpp"

const int WIN_W = 800;
const int WIN_H = 600;

Camera cam1 = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f }; // First Person
Camera cam2 = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f }; // Rear View
Camera cam3 = { 0.0f, 20.0f, 0.0f, 0.0f, 0.0f, 0.0f }; // Bird’s Eye

bool rearCamOn = true;
bool sceneCamOn = true;
bool camSwitch = false;

#define PI 3.14159265f // used for converting angles to radians

// Randomness per robot for dance animations
float robotSpeeds[5];   // multiplier for how fast each robot animates (0.8 - 1.4)
float robotOffsets[5];  // phase offset in degrees (0 - 359)
int robotTypes[5];      // selects which distinct animation (0 - 4)
Vector3 robotPositions[5];

float cameraMS = 0.1f;

bool dancing = false;
bool groupDance = true;
int danceAngle = 0;    // current frame of animation
float torsoBounce = 0.05f * sin(danceAngle * 3.14 / 180.0f);

bool axies = false;
State state = SOLID;
bool clear = false;
Vector3 globalRot(0.0, 0.0, 0.0);
bool ortho = true;

Vector3 torsoColor = getColor(BLUE);
Vector3 headColor = getColor(YELLOW);
Vector3 armColor = getColor(GREEN);
Vector3 legColor = getColor(RED);

//
//////////// BONUS - Improved Confetti (replace previous confetti code)
//
struct Particle {
	float x, y, z;        // position
	float r, g, b, a;     // color + alpha
	float vx, vy, vz;     // velocity
	float life;           // lifetime
	int emitter;          // which robot it spawns from (0..4)
	float seed;           // per-particle seed to keep consistent
};

const int MAX_PARTICLES = 100; // total particles
Particle particles[MAX_PARTICLES];
bool confettiActive = false;

// confetti constants
const float CONFETTI_SPAWN_HEIGHT = 5.5f;   // spawn above robot
const float CONFETTI_SPAWN_RADIUS = 0.8f;   // how far from robot center confetti spawns
const float CONFETTI_MIN_VY = -0.008f;      // initial vertical speed
const float CONFETTI_MAX_VY = -0.02f;
const float CONFETTI_GRAVITY = 0.00012f;    // gravity
const float CONFETTI_DRIFT_AMPL = 0.012f;   // horizontal drift
const float CONFETTI_POINT_SIZE = 4.5f;     // drawn size
const float CONFETTI_ALPHA_MIN = 0.45f;
const float CONFETTI_ALPHA_MAX = 0.85f;

void initConfetti() {
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		Particle& p = particles[i];

		// choose a robot emitter so confetti comes from robots
		p.emitter = rand() % 5;

		// spawn roughly above that robot with slight random offset
		float rx = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * CONFETTI_SPAWN_RADIUS;
		float rz = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * CONFETTI_SPAWN_RADIUS;

		// set initial coordinates
		p.x = robotPositions[p.emitter].x + rx;
		p.z = robotPositions[p.emitter].z + rz;
		p.y = robotPositions[p.emitter].y + CONFETTI_SPAWN_HEIGHT + ((rand() % 100) / 200.0f); // small vertical jitter

		// randomized initial velocities
		p.vx = ((rand() % 100) / 100.0f) * 0.02f - 0.01f; // -0.01 .. 0.01
		p.vz = ((rand() % 100) / 100.0f) * 0.02f - 0.01f;
		p.vy = -(CONFETTI_MIN_VY + ((rand() % 100) / 100.0f) * (CONFETTI_MAX_VY - CONFETTI_MIN_VY));

		// color + alpha
		// always keeps a bright color
		p.r = 0.5f + (rand() % 50) / 100.0f; // 0.5..1.0
		p.g = 0.5f + (rand() % 50) / 100.0f;
		p.b = 0.5f + (rand() % 50) / 100.0f;
		p.a = CONFETTI_ALPHA_MIN + ((rand() % 100) / 100.0f) * (CONFETTI_ALPHA_MAX - CONFETTI_ALPHA_MIN);

		// lifetime and seed set
		p.life = 1.0f;
		p.seed = (rand() % 1000) / 1000.0f * (2.0f*PI); // 0..2PI phase for each angle
	}
}

// used for updating in main()
void updateConfetti() {
	if (!confettiActive) return;

	// time based phase for movement
	float t = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // seconds

	for (int i = 0; i < MAX_PARTICLES; ++i) {
		Particle& p = particles[i];

		// adds drift to confetti, used to help smooth jumps between updates
		float driftX = sinf(t * 0.8f + p.seed) * CONFETTI_DRIFT_AMPL;
		float driftZ = cosf(t * 0.6f + p.seed * 1.3f) * (CONFETTI_DRIFT_AMPL * 0.8f);

		p.x += p.vx + driftX * 0.02f;
		p.z += p.vz + driftZ * 0.02f;
		p.y += p.vy; // only affected by gravity

		// gravity
		p.vy -= CONFETTI_GRAVITY;

		// if a particle is below the floor, it is recreated and starts again
		// this is is a copy of the above INIT function
		if (p.y < -1.0f) { // below floor
			p.emitter = rand() % 5;
			float rx = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * CONFETTI_SPAWN_RADIUS;
			float rz = ((rand() % 100) / 100.0f * 2.0f - 1.0f) * CONFETTI_SPAWN_RADIUS;
			p.x = robotPositions[p.emitter].x + rx;
			p.z = robotPositions[p.emitter].z + rz;
			p.y = robotPositions[p.emitter].y + CONFETTI_SPAWN_HEIGHT + ((rand() % 100) / 200.0f);

			p.vx = ((rand() % 100) / 100.0f) * 0.02f - 0.01f;
			p.vz = ((rand() % 100) / 100.0f) * 0.02f - 0.01f;
			p.vy = -(CONFETTI_MIN_VY + ((rand() % 100) / 100.0f) * (CONFETTI_MAX_VY - CONFETTI_MIN_VY));

			p.r = 0.5f + (rand() % 50) / 100.0f;
			p.g = 0.5f + (rand() % 50) / 100.0f;
			p.b = 0.5f + (rand() % 50) / 100.0f;
			p.a = CONFETTI_ALPHA_MIN + ((rand() % 100) / 100.0f) * (CONFETTI_ALPHA_MAX - CONFETTI_ALPHA_MIN);

			p.life = 1.0f;
			p.seed = (rand() % 1000) / 1000.0f * 6.2831853f;
		}
	}
}

// Confetti is drawn for each camera to keep the same scene
void drawConfetti() {
	if (!confettiActive) return;

	// save current enable state, color, and point size
	glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_POINT_BIT);

	// want confetti to render as bright translucent points regardless of lighting
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(CONFETTI_POINT_SIZE);
	glBegin(GL_POINTS);
	for (int i = 0; i < MAX_PARTICLES; ++i) {
		Particle& p = particles[i];
		glColor4f(p.r, p.g, p.b, p.a);
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd();

	// restore
	glDisable(GL_BLEND);
	glPopAttrib();
}

//
////////////// BONUS END
//


// camera input
void specialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT: // rotate camera left
		cam1.angle -= 0.05f;
		cam1.lx = sin(cam1.angle);
		cam1.lz = -cos(cam1.angle);
		break;
	case GLUT_KEY_RIGHT: // rotate camera right
		cam1.angle += 0.05f;
		cam1.lx = sin(cam1.angle);
		cam1.lz = -cos(cam1.angle);
		break;
	case GLUT_KEY_UP: // move camera forward
		cam1.x += cam1.lx * cameraMS;
		cam1.z += cam1.lz * cameraMS;
		break;
	case GLUT_KEY_DOWN: // move camera backwards
		cam1.x -= cam1.lx * cameraMS;
		cam1.z -= cam1.lz * cameraMS;
		break;

	case GLUT_KEY_F1: // toggle rear view
		rearCamOn = !rearCamOn;
		break;
	case GLUT_KEY_F2: // toggle bird’s-eye
		sceneCamOn = !sceneCamOn;
		break;
	case GLUT_KEY_F3: // toggle between camera types
		camSwitch = !camSwitch;
		break;
	}

	// Keep rear camera synced
	cam2 = cam1;

	glutPostRedisplay();
}

void keyboardInput(unsigned char input, int x, int y)
{
	switch (input)
	{
	case 'a': // displays x(red), y(green), z(blue) axies
		axies = !axies;
		break;
	case 'c': // keeps display clear by preventing drawing
		clear = !clear;
		break;

	case 'd': // toggle dancing
		dancing = !dancing;
		if (dancing) {
			glutTimerFunc(0, danceTimer, 0);
			PlaySound(TEXT("dance1.wav"), NULL, SND_FILENAME | SND_ASYNC);
		}
		else {
			PlaySound(NULL, 0, 0);
			//
			//////////// BONUS
			//
			confettiActive = false; // stop confetti automatically when dance stops
			//
		}
		break;

	//
	//////////// BONUS
	//
	case 'f': // toggle confetti (only works if dancing)
		if (dancing) {
			confettiActive = !confettiActive;
			if (confettiActive) {
				initConfetti();
			}
		}
		break;
	//
	case 'i':  // toggle between group and individual dancing
		groupDance = !groupDance;
		break;

	case 'p': // sets vertex state
		state = VERTEX;
		break;
	case 'w': // sets wire state
		state = WIRE;
		break;
	case 's': // sets solid state (default)
		state = SOLID;
		break;

	case 'q': // exits program
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay(); // for instantly updating graphics
}

void drawAxies()
{
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); // X axis - Color
	glVertex3f(0, 0, 0); glVertex3f(100, 0, 0); // Start and end
	glColor3f(0, 1, 0); // Y axis
	glVertex3f(0, 0, 0); glVertex3f(0, 100, 0);
	glColor3f(0, 0, 1); // Z axis
	glVertex3f(0, 0, 0); glVertex3f(0, 0, 100);
	glEnd();
}

// Basic 100 x 100 dark-grey floor
void drawGroundPlane() {
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_QUADS);
	glVertex3f(-100.0f, -1.5f, -100.0f);
	glVertex3f(-100.0f, -1.5f, 100.0f);
	glVertex3f(100.0f, -1.5f, 100.0f);
	glVertex3f(100.0f, -1.5f, -100.0f);
	glEnd();

}

// draws head, body, legs, and arms
// Draws one robot using a localAngle (degrees) and a dance type to vary animations
void drawRobot(float localAngle, int type)
{
	// compute torso bounce using sine of localAngle in radians
	float torsoBounceLocal = 0.05f * sinf(localAngle * PI / 180.0f);

	// rotation of each body part for unique dances
	float headYrot = 0.0f;
	float leftArmXrot = 0.0f;
	float rightArmXrot = 0.0f;
	float bodyYaw = 0.0f;
	float lateralShift = 0.0f;
	float leftLegXrot = 0.0f;
	float rightLegXrot = 0.0f;

	// different types of animations
	float swing = (sinf(localAngle * PI / 180.0f) * 30.0f); // -30..30 deg
	float kick = (sinf(localAngle * PI / 180.0f) * 40.0f); // -40..40 deg
	float bob = (sinf(localAngle * PI / 180.0f) * 0.1f);  // -0.1..0.1 units

	switch (type) {
	case 0: // arms swing, head slight bob
		leftArmXrot = swing;
		rightArmXrot = -swing;
		headYrot = (sinf(localAngle * PI / 180.0f * 0.5f) * 10.0f); // gentle head turn
		leftLegXrot = -swing * 0.5f;
		rightLegXrot = swing * 0.5f;
		break;
	case 1: // head-bob, torso bounce
		headYrot = (sinf(localAngle * PI / 180.0f * 2.0f) * 25.0f);
		leftArmXrot = swing * 0.3f;
		rightArmXrot = -swing * 0.3f;
		break;
	case 2: // haevy leg-kick
		leftLegXrot = kick;
		rightLegXrot = -kick;
		leftArmXrot = kick * 0.25f;
		rightArmXrot = -kick * 0.25f;
		break;
	case 3: // spin in place
		bodyYaw = fmodf(localAngle * 2.0f, 360.0f);
		leftArmXrot = 20.0f * sinf(localAngle * PI / 180.0f);
		rightArmXrot = -20.0f * sinf(localAngle * PI / 180.0f);
		break;
	case 4: // lateral shift, arm movement
		lateralShift = bob * 0.5f;
		leftArmXrot = swing * 0.4f;
		rightArmXrot = -swing * 0.4f;
		break;
	default: // arm swing
		leftArmXrot = swing;
		rightArmXrot = -swing;
		break;
	}

	// Start drawing the robot using the computed transforms
	glPushMatrix();

	// Apply lateral shuffle
	if (lateralShift != 0.0f) glTranslatef(lateralShift, 0.0f, 0.0f);

	// Dancing values from before implemented into body parts
	// Torso (blue)
	createObject(CUBE,
		Vector3(0.0f, torsoBounceLocal, 0.0f),
		Vector3(0.0f, bodyYaw, 0.0f),
		Vector3(0.8f, 1.0f, 0.4f),
		torsoColor);

	// Head (yellow) - rotated around Y
	createObject(CUBE,
		Vector3(0.0f, 0.8f, 0.0f),
		Vector3(0.0f, headYrot, 0.0f),
		Vector3(0.6f, 0.6f, 0.6f),
		headColor);

	// Left Arm (green) - rotation about X axis
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

void drawRobotAtRandom(int i) {
	glPushMatrix();
	glTranslatef(robotPositions[i].x, 0.0f, robotPositions[i].z);

	float localAngle;
	int danceType;

	if (groupDance) { // All robots default to first dance
		localAngle = danceAngle;
		danceType = 0;
	}
	else { // each robot has a unique dance
		localAngle = danceAngle * robotSpeeds[i] + robotOffsets[i];
		danceType = robotTypes[i];
	}

	drawRobot(localAngle, danceType);
	glPopMatrix();
}


void renderSceneFromCamera(Camera cam, int viewW, int viewH)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (double)viewW / (double)viewH, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// special case to change angle for bird's eye
	if (cam.y > 5.0f) {
		gluLookAt(cam.x, cam.y, cam.z,
			cam.x, 0.0f, cam.z,
			0.0f, 0.0f, -1.0f);     
	}
	else { // default camera
		gluLookAt(cam.x, cam.y, cam.z,
			cam.x + cam.lx, cam.y, cam.z + cam.lz,
			0.0f, 1.0f, 0.0f);
	}

	// draws robots and ground for each camera to keep same scene for all
	drawGroundPlane();
	for (int i = 0; i < 5; i++) drawRobotAtRandom(i);
	if (axies) drawAxies();
	//
	//////////// BONUS
	//
	drawConfetti();
	//
}

void MyDisplay() {
	glViewport(0, 0, WIN_W, WIN_H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// clear screen
	if (clear) {
		glutSwapBuffers();
		return;
	}

	if (camSwitch) { // special case of cameras (Bird's Eye POV is primary)
		clearViewportArea(0, 0, WIN_W, WIN_H);

		glViewport(0, 0, WIN_W, WIN_H);
		renderSceneFromCamera(cam3, WIN_W, WIN_H);

		// First Person POV camera - Top Right
		if (sceneCamOn) {
			int w = WIN_W / 4;
			int h = WIN_H / 4;
			int x = WIN_W - w;
			int y = WIN_H - h;

			clearViewportArea(x, y, w, h);
			glViewport(x, y, w, h);
			renderSceneFromCamera(cam1, w, h);
		}

		// Rear View POV camera - Top Left
		if (rearCamOn) {
			int w = WIN_W / 4;
			int h = WIN_H / 4;
			int x = 0;
			int y = WIN_H - h;
			clearViewportArea(x, y, w, h);
			glViewport(x, y, w, h);

			Camera rear = cam2;
			rear.angle += 3.14159f;
			rear.lx = sin(rear.angle);
			rear.lz = -cos(rear.angle);
			renderSceneFromCamera(rear, w, h);
		}
	}
	else { // default case of cameras (First Person POV is primary)
		clearViewportArea(0, 0, WIN_W, WIN_H);
		glViewport(0, 0, WIN_W, WIN_H);
		renderSceneFromCamera(cam1, WIN_W, WIN_H);

		// Rear View POV camera - Top Left
		if (rearCamOn) {
			int w = WIN_W / 4;
			int h = WIN_H / 4;
			int x = 0;
			int y = WIN_H - h;
			clearViewportArea(x, y, w, h);
			glViewport(x, y, w, h);

			Camera rear = cam2;
			rear.angle += 3.14159f;
			rear.lx = sin(rear.angle);
			rear.lz = -cos(rear.angle);
			renderSceneFromCamera(rear, w, h);
		}

		// Bird's Eye POV camera - Top Right
		if (sceneCamOn) {
			int w = WIN_W / 4;
			int h = WIN_H / 4;
			int x = WIN_W - w;
			int y = WIN_H - h;
			clearViewportArea(x, y, w, h);
			glViewport(x, y, w, h);

			Camera bird;
			bird.x = 0.0f; bird.y = 20.0f; bird.z = 0.01f;
			bird.lx = 0.0f; bird.lz = 0.0f;
			renderSceneFromCamera(bird, w, h);
		}
	}

	glutSwapBuffers();
}


int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Andrew Trautzsch, 811198871");

	initRobots(); // sets the robot posistion for all cameras

	// instruction helper functions
	printInstructions();

	// OpenGL setup
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	//
	//////////// BONUS
	//
	glutIdleFunc([]() {
		if (confettiActive) {
			updateConfetti();
			glutPostRedisplay(); // redraw frame continuously
		}
		});
	//

	// Callbacks
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(keyboardInput);
	glutSpecialFunc(specialKeys); // For camera movement and switching
	
	glutMainLoop();
	return 0;
}
