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

// creates angle, projection, and color menus
void createMenus() {
	// Rotation submenus
	int angles[] = { 0,30,60,90,120,150,180,210,240,270,300,330,360 };
	int xMenu = glutCreateMenu(rotateX);
	for (int a : angles) glutAddMenuEntry(std::to_string(a).c_str(), a);
	int yMenu = glutCreateMenu(rotateY);
	for (int a : angles) glutAddMenuEntry(std::to_string(a).c_str(), a);
	int zMenu = glutCreateMenu(rotateZ);
	for (int a : angles) glutAddMenuEntry(std::to_string(a).c_str(), a);

	// Projection submenu
	int projectionMenu = glutCreateMenu(menuProjection);
	glutAddMenuEntry("Orthographic", 1);
	glutAddMenuEntry("Perspective", 2);

	// Body part color submenus
	Color colors[] = { RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE };
	int torsoMenu = glutCreateMenu(setTorsoColor);
	for (Color c : colors) glutAddMenuEntry(getColorName(c), c);
	int headMenu = glutCreateMenu(setTorsoColor);
	for (Color c : colors) glutAddMenuEntry(getColorName(c), c);
	int armMenu = glutCreateMenu(setTorsoColor);
	for (Color c : colors) glutAddMenuEntry(getColorName(c), c);
	int legMenu = glutCreateMenu(setTorsoColor);
	for (Color c : colors) glutAddMenuEntry(getColorName(c), c);

	// Grouped Body Colors menu
	int bodyColorMenu = glutCreateMenu([](int) {});
	glutAddSubMenu("Torso", torsoMenu);
	glutAddSubMenu("Head", headMenu);
	glutAddSubMenu("Arms", armMenu);
	glutAddSubMenu("Legs", legMenu);

	// Main menu
	int mainMenu = glutCreateMenu([](int) {});
	glutAddSubMenu("Rotate X", xMenu);
	glutAddSubMenu("Rotate Y", yMenu);
	glutAddSubMenu("Rotate Z", zMenu);
	glutAddSubMenu("Projection", projectionMenu);
	glutAddSubMenu("Body Colors", bodyColorMenu);

	// Attach to right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// used for rotation menu
void rotateX(int input) { globalRot.x = input; glutPostRedisplay(); }
void rotateY(int input) { globalRot.y = input; glutPostRedisplay(); }
void rotateZ(int input) { globalRot.z = input; glutPostRedisplay(); }

// used for color menu
void setTorsoColor(int c) { torsoColor = getColor((Color)c); glutPostRedisplay(); }
void setHeadColor(int c) { headColor = getColor((Color)c); glutPostRedisplay(); }
void setArmColor(int c) { armColor = getColor((Color)c); glutPostRedisplay(); }
void setLegColor(int c) { legColor = getColor((Color)c); glutPostRedisplay(); }

// used for dance animation
void danceTimer(int value) {
	if (dancing) {
		danceAngle = (danceAngle + 5) % 360;  // animate arms/legs rotation
		glutPostRedisplay();
		glutTimerFunc(50, danceTimer, 0);     // call again in ~50 ms
	}
}

// used for projection menu
void menuProjection(int option) {
	ortho = (option == 1);
	glutPostRedisplay();
}

//
//////////// BONUS
//
// helper functions used for bonus

// plays upon pushing h and program start
void playHello() {
	PlaySound(TEXT("hello.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

// plays upon pushing j and on dance
void playDance() {
	PlaySound(TEXT("dance.wav"), NULL, SND_FILENAME | SND_ASYNC);
}

// plays upon pushing k and on program exit
void playBye() {
	PlaySound(TEXT("bye.wav"), NULL, SND_FILENAME | SND_ASYNC);
}
//
