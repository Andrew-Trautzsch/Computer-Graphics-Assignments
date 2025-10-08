/*
Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 1 - Humanoid Robot

BONUS:
Added 3 different voice audio clips, they can be played with keyboard input or they will play passively
	h - hello audio
	j - dance audio
	l - goodbye audio

hello will automatically play at program start, dance at dance start, and goodbye and program termination (with keyboard input q)

all sections that have bonus related code are marked with

//
//////////// BONUS
//
	(content)
//
*/

#include "helper.hpp"

Camera cam1 = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f }; // First Person
Camera cam2 = { 0.0f, 1.0f, 5.0f, 0.0f, -1.0f, 0.0f }; // Rear View
Camera cam3 = { 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f }; // Bird’s Eye

Vector3 robotPositions[5];

float cameraMS = 0.1f;

bool dancing = false;  // toggled with 'd'
int danceAngle = 0;    // current frame of animation
float torsoBounce = 0.05f * sin(danceAngle * 3.14 / 180.0f);

bool axies = false;
State state = SOLID;
bool clear = false;
bool BnW = false;
Vector3 globalRot(0.0, 0.0, 0.0);
float zoom = 1.0f;
bool ortho = true;
bool musicPlaying = false;

bool rearCamOn = true;
bool sceneCamOn = true;
bool camSwitch = false;

Vector3 torsoColor = getColor(BLUE);
Vector3 headColor = getColor(YELLOW);
Vector3 armColor = getColor(GREEN);
Vector3 legColor = getColor(RED);

void specialKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_LEFT:
		cam1.angle -= 0.05f;
		cam1.lx = sin(cam1.angle);
		cam1.lz = -cos(cam1.angle);
		break;
	case GLUT_KEY_RIGHT:
		cam1.angle += 0.05f;
		cam1.lx = sin(cam1.angle);
		cam1.lz = -cos(cam1.angle);
		break;
	case GLUT_KEY_UP:
		cam1.x += cam1.lx * cameraMS;
		cam1.z += cam1.lz * cameraMS;
		break;
	case GLUT_KEY_DOWN:
		cam1.x -= cam1.lx * cameraMS;
		cam1.z -= cam1.lz * cameraMS;
		break;

	case GLUT_KEY_F1:
		rearCamOn = !rearCamOn;     // toggle rear view
		break;
	case GLUT_KEY_F2:
		sceneCamOn = !sceneCamOn;   // toggle bird’s-eye
		break;
	case GLUT_KEY_F3:
		camSwitch = !camSwitch;     // toggle between FPV and ESV
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
		//
		//////////// BONUS
		//
		playDance();
		//
		if (dancing) {
			glutTimerFunc(0, danceTimer, 0);  // start animation loop

			// only play music if user has enabled it
			if (musicPlaying) {
				PlaySound(TEXT("dance1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			}
		}
		else {
			// stop music when dancing stops
			PlaySound(NULL, 0, 0);
		}
		break;
	case 'm':
		musicPlaying = !musicPlaying;
		if (musicPlaying && dancing) {
			PlaySound(TEXT("dance1.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
		}
		else {
			PlaySound(NULL, 0, 0); // stop
		}
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
	//
	//////////// BONUS
	//
	// manual keyboard inputs for audio

	case 'h': // say hello
		playHello();
		break;
	case 'j': // say dance
		playDance();
		break;
	case 'k': // say bye
		playBye();
		break;
	//
	case 'q': // exits program
		//
		////////////// BONUS
		//
		playBye();
		//
		Sleep(1500);
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay(); // for instantly updating graphics
}

/*
Input: 0 = left btn, 2 = right btn, 3 = scroll up, 4 = scroll down
/////
Toggle: 0 = down, 1 = up
*/
void mouseInput(int button, int state, int x, int y) {
	if (state != GLUT_DOWN) return; // only handle press, not release

	switch (button) {
	case GLUT_LEFT_BUTTON: // toggles color mode
		BnW = !BnW;
		break;
	// right button un-needed as it is handled in createMenus()
	case 3: // scroll up
		zoom *= 0.9f;
		break;
	case 4: // scroll down
		zoom *= 1.1f;
		break;
	}
	glutPostRedisplay();
}

void drawAxies()
{
	glBegin(GL_LINES);
	glColor3f(1, 0, 0); // X axis - Color
	glVertex3f(0, 0, 0); glVertex3f(10, 0, 0); // Start and end
	glColor3f(0, 1, 0); // Y axis
	glVertex3f(0, 0, 0); glVertex3f(0, 10, 0);
	glColor3f(0, 0, 1); // Z axis
	glVertex3f(0, 0, 0); glVertex3f(0, 0, 10);
	glEnd();
}

void createObject(Shape type, Vector3 position, Vector3 rotation, Vector3 scale, Vector3 color)
{
	// If in wireframe mode, override color to white
	if (state == WIRE)
		color = Vector3(1.0f, 1.0f, 1.0f);

	// starts matrix
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

// draws head, body, legs, and arms
void drawRobot()
{
	// Torso (blue)
	createObject(CUBE,						 // type
		Vector3(0.0f, torsoBounce, 0.0f),    // posistion
		Vector3(0, 0, 0),				     // rotation
		Vector3(0.8f, 1.0f, 0.4f),			 // scale
		BnW ? getColor(WHITE) : torsoColor); // color

	// Head (yellow)
	createObject(CUBE,
		Vector3(0.0f, 0.8f, 0.0f),
		Vector3(0, danceAngle, 0),  // rotate on Y axis
		Vector3(0.6f, 0.6f, 0.6f),
		BnW ? getColor(WHITE) : headColor);

	// Left Arm (green)
	createObject(CUBE,
		Vector3(-0.55f, 0.0f, 0.0f),
		Vector3(danceAngle % 60 - 30, 0, 0), // oscillates between -30 and 30
		Vector3(0.3f, 1.0f, 0.3f),
		BnW ? getColor(WHITE) : armColor);

	// Right Arm (green)
	createObject(CUBE,
		Vector3(0.55f, 0.0f, 0.0f),
		Vector3(-(danceAngle % 60 - 30), 0, 0),
		Vector3(0.3f, 1.0f, 0.3f),
		BnW ? getColor(WHITE) : armColor);

	// Left Leg (red)
	createObject(CUBE,
		Vector3(-0.2f, -1.0f, 0.0f),
		Vector3(0, 0, 0),
		Vector3(0.3f, 1.0f, 0.3f),
		BnW ? getColor(WHITE) : legColor);

	// Right Leg (red)
	createObject(CUBE,
		Vector3(0.2f, -1.0f, 0.0f),
		Vector3(0, 0, 0),
		Vector3(0.3f, 1.0f, 0.3f),
		BnW ? getColor(WHITE) : legColor);
}

void drawGroundPlane() {
	glColor3f(0.3f, 0.7f, 0.3f);
	glBegin(GL_QUADS);
	glVertex3f(-20.0f, -1.0f, -20.0f);
	glVertex3f(-20.0f, -1.0f, 20.0f);
	glVertex3f(20.0f, -1.0f, 20.0f);
	glVertex3f(20.0f, -1.0f, -20.0f);
	glEnd();
}

void initRobots() {
	srand(time(NULL));
	for (int i = 0; i < 5; i++) {
		robotPositions[i] = Vector3((rand() % 20) - 10, 0.0f, (rand() % 20) - 10);
	}
}

void drawRobotAtRandom(int i) {
	glPushMatrix();
	glTranslatef(robotPositions[i].x, 0.0f, robotPositions[i].z);
	drawRobot();
	glPopMatrix();
}

void renderSceneFromCamera(Camera cam)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 800.0 / 600.0, 0.1, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(cam.x, cam.y, cam.z,
		cam.x + cam.lx, cam.y, cam.z + cam.lz,
		0.0f, 1.0f, 0.0f);

	drawGroundPlane();
	for (int i = 0; i < 5; i++) drawRobotAtRandom(i); // multiple robots
	if (axies) drawAxies();
}

void MyDisplay() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// If the user toggled screen clearing (c key)
	if (clear) {
		glutSwapBuffers();
		return;
	}

	// === MAIN VIEW ===
	if (camSwitch) {
		// F3 active  Bird’s-eye only
		glViewport(0, 0, 800, 600);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, 800.0 / 600.0, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 20.0f, 0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		drawGroundPlane();
		for (int i = 0; i < 5; i++) drawRobotAtRandom(i);
		if (axies) drawAxies();
	}
	else {
		// Normal first-person view
		glViewport(0, 0, 800, 600);
		renderSceneFromCamera(cam1);
	}

	// === REAR VIEW (F1 toggle) ===
	if (rearCamOn) {
		glViewport(0, 450, 200, 150);
		Camera rear = cam2;
		rear.angle += 3.14159f;
		rear.lx = sin(rear.angle);
		rear.lz = -cos(rear.angle);
		renderSceneFromCamera(rear);
	}

	// === BIRD’S-EYE (F2 toggle) ===
	if (sceneCamOn) {
		glViewport(600, 450, 200, 150);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60.0, 800.0 / 600.0, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0.0f, 20.0f, 0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f);
		drawGroundPlane();
		for (int i = 0; i < 5; i++) drawRobotAtRandom(i);
		if (axies) drawAxies();
	}

	glutSwapBuffers();
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Andrew Trautzsch, 811198871");

	// menu and instruction helper functions
	createMenus();
	printInstructions();

	// OpenGL setup
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	// Callbacks
	glutDisplayFunc(MyDisplay);
	glutKeyboardFunc(keyboardInput);
	glutMouseFunc(mouseInput);
	glutSpecialFunc(specialKeys); // For camera movement

	//
	//////////// BONUS
	//
	playHello();
	//
	
	glutMainLoop();
	return 0;
}
