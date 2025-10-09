Andrew Trautzsch - 811198871
Animation and Game Design - Assignment 2 - Humanoid Robot

Keep a files in same folder and run .exe file to execute program, use the .sln file to open and access code and comments

Controls for program:

================= Robot Controls =================
Keyboard Controls:
  p : Display vertex-only model of robots
  w : Display wireframe model of robots
  s : Display solid model of robots (default)
  c : Toggle screen clearing (black background only)
  a : Toggle axes display (X=Red, Y=Green, Z=Blue)
  d : Toggle dancing animation on/off
  i : Toggle between group dancing and individual dancing
  f : Toggle confetti particle system (only works while dancing)
  q : Quit the program

Camera Controls (Arrow Keys):
  UP    : Move camera forward
  DOWN  : Move camera backward
  LEFT  : Rotate camera left
  RIGHT : Rotate camera right

Camera Views (Function Keys):
  F1 : Toggle rear view camera (top-left viewport)
  F2 : Toggle birds-eye camera (top-right viewport)
  F3 : Switch main display between First Person and BirdÆs Eye view

==================================================


BONUS:
Added a confetti particle system. Creates particles with pos and veloctiy above robots, which spreads and falls, once hitting y = -1, resets to a new posistion and repeats
Can be toggled while dancing with 'f', set max of 100 particles at a time

all sections that have bonus related code are marked with

//
//////////// BONUS
//
	(content)
//