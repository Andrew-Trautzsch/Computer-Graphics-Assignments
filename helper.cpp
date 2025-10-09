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
	std::cout << "================= Assignment 2: Moving Cameras & Humanoid Robots =================\n";
	std::cout << "Author: Andrew Trautzsch (811198871)\n";
	std::cout << "-------------------------------------------------------------------------------\n";
	std::cout << "Keyboard Controls:\n";
	std::cout << "  p : Display vertex-only model of robots\n";
	std::cout << "  w : Display wireframe model of robots\n";
	std::cout << "  s : Display solid model of robots (default)\n";
	std::cout << "  c : Toggle screen clearing (black background only)\n";
	std::cout << "  a : Toggle axes display (X=Red, Y=Green, Z=Blue)\n";
	std::cout << "  d : Toggle dancing animation on/off\n";
	std::cout << "  i : Toggle between group dancing and individual dancing\n";
	std::cout << "  f : Toggle confetti particle system (only works while dancing)\n";
	std::cout << "  q : Quit the program\n\n";

	std::cout << "Camera Controls (Arrow Keys):\n";
	std::cout << "  UP    : Move camera forward\n";
	std::cout << "  DOWN  : Move camera backward\n";
	std::cout << "  LEFT  : Rotate camera left\n";
	std::cout << "  RIGHT : Rotate camera right\n\n";

	std::cout << "Camera Views (Function Keys):\n";
	std::cout << "  F1 : Toggle rear view camera (top-left viewport)\n";
	std::cout << "  F2 : Toggle bird’s-eye camera (top-right viewport)\n";
	std::cout << "  F3 : Switch main display between First Person and Bird’s Eye view\n\n";

	std::cout << "-------------------------------------------------------------------------------\n";
	std::cout << "Bonus Feature:\n";
	std::cout << "  Confetti Particle System:\n";
	std::cout << "   - Can be toggled during dancing with the 'f' key\n";
	std::cout << "   - Spawns colorful confetti above robots that falls and resets\n";
	std::cout << "   - Automatically turns off when dancing stops\n\n";
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