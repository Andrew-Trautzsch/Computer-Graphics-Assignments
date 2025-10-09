/*
This file contains "helper" functions that assited main functions in main.cpp
The printInstructions() and printMenus() functions are here as well to reduce space in main.cpp
*/

#include "helper.hpp"

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

// instructions on using program
void printInstructions() {
	std::cout << "================= Robot Controls =================\n";
	std::cout << "Keyboard Controls:\n";
	std::cout << "  p : Vertex-only model\n";
	std::cout << "  w : Wireframe model\n";
	std::cout << "  s : Solid model\n";
	std::cout << "  c : Clear screen toggle\n";
	std::cout << "  a : Toggle axes on/off\n";
	std::cout << "  d : Toggle dancing animation on/off\n";
	std::cout << "  m : Toggle music on/off\n";
	std::cout << "  h : Play hello audio\n";
	std::cout << "  j : Play dance audio\n";
	std::cout << "  k : Play goodbye audio\n";
	std::cout << "  q : Quit program\n\n";

	std::cout << "Mouse Controls:\n";
	std::cout << "  Right click : Open popup menu\n";
	std::cout << "  Left click  : Toggle between B&W and Color\n";
	std::cout << "  Middle scroll : Zoom in/out\n\n";

	std::cout << "Menu Options:\n";
	std::cout << "  - Rotate robot (angles 30-360) along X, Y, Z axes\n";
	std::cout << "  - Projection: Orthographic (default) or Perspective\n";
	std::cout << "  - Change body part colors (Head, Torso, Arms, Legs)\n\n";

	std::cout << "==================================================\n";
}

// used for dance animation
void danceTimer(int value) {
	if (dancing) {
		danceAngle = (danceAngle + 5) % 360;  // animate arms/legs rotation
		glutPostRedisplay();
		glutTimerFunc(50, danceTimer, 0);     // call again in ~50 ms
	}
}

// helper function for robot body creation
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

// Used to prevent overlap of primary camera and corner cameras
void clearViewportArea(int x, int y, int w, int h) {
	// Clears only the rectangle defined by (x,y,w,h)
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, y, w, h);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);
}

// initializes robot randomness (posistion and dance properties)
void initRobots() {
	srand((unsigned int)time(NULL));
	for (int i = 0; i < 5; i++) {
		// random positions in [-10,10)
		robotPositions[i] = Vector3((rand() % 20) - 10, 0.0f, (rand() % 20) - 10);

		// Randomize dance parameters:
		// Speed
		robotSpeeds[i] = 0.8f + (rand() % 61) / 100.0f;

		// Phase offset
		robotOffsets[i] = (float)(rand() % 360);

		// Dance type
		robotTypes[i] = rand() % 5;
	}
}