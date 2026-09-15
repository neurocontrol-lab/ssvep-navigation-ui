#pragma once
#pragma comment (lib, "glew32s.lib")
#pragma comment(lib, "winmm.lib") 
#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS


#define GLEW_STATIC
#include <GL/glew.h>
#include <windows.h> //Seems necessary for GLUT
#include <mmsystem.h>
#include <GL/glu.h> 
#include <GL/glut.h>
#include <GL\freeglut.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include "Constants.h"
#include "mazeParser.h"
#include "helperMethods.h"
#include "drawHUD.h"
#include "textures.h"
#include <sstream>
#include <functional>
#include <string>
#include <ctime>

using namespace std;


int startTime = clock(), endTime = clock() + 185000;
int sec, mn = 0, hour = 0;
bool gameOver = false, youWon = false;

// Track if keys are currently held down (Supports both lower & upper case)
bool keys[256] = { false };
bool specialKeys[256] = { false };
bool flickerEnabled = true;
unsigned long long stimulusFrame = 0;
double displayRefreshHz = 0.0;

// Jump configurations (Adjust these values to change jump feel)
const float GRAVITY = -28.0f;       // Acceleration pulling you down
const float JUMP_VELOCITY = 25.0f;   // Initial upward speed of the jump
const float FLOOR_HEIGHT = -HALF_CUBE;    // Your default standing camera height
const float MOUSE_SENSITIVITY = 0.001f; //Adjust this to your liking

// Jump state trackers
float vertical_velocity = 0.0f;
bool is_jumping = false;

bool victorySoundPlayed = false;

// Delta Time variables
int oldTimeSinceStart = 0;

// File-level variables... these are all position-state / input state variables. OpenGL 
//  callbacks with defined signatures must edit these variables, so there's no easy 
//  alternative to giving them file-level scope.
static GLfloat x_at = START_X_AT;
static GLfloat y_at = START_Y_AT;
static GLfloat rot_x = M_PI_2;
static GLint xin = 0, yin = 0;
static GLfloat camera_y = START_CAMERA_Y;
static GLfloat rot_y = -M_PI_2;

//App-level "init" function
void initgl(GLint width, GLint height) 
{
 glClearColor(0.0f, 0.0f, 0.0f, 0.0f); 
 glClearDepth(1.0);
 glEnable(GL_DEPTH_TEST);
 glFrontFace(GL_CCW);
 glShadeModel(GL_SMOOTH); 
 glMatrixMode(GL_PROJECTION);
 gluPerspective(VIEW_FIELD,(GLfloat)width/(GLfloat)height,NEAR_Z,FAR_Z); 
 glMatrixMode(GL_MODELVIEW);
}

/* The function called when our window is resized (not supported with our textur. sys.)*/
void resizer(GLint width, GLint height)
{ 
 if(width!=windowwidth() || height!=windowheight()) exit(0); 
}


bool collide() //Is player in a state of collision?
{
 int x,y;

 if (gameOver || youWon) return false;

 //Got out of the maze!!!
 if(x_at>=MAZE_EXTREME_LEFT-COLLIDE_MARGIN - HALF_CUBE && 
   x_at<=MAZE_EXTREME_LEFT+ XSIZE* FULL_CUBE+ HALF_CUBE +COLLIDE_MARGIN)
 {
  if( y_at<=MAZE_EXTREME_TOP+COLLIDE_MARGIN - HALF_CUBE ||
    y_at>=MAZE_EXTREME_TOP+YSIZE*FULL_CUBE + HALF_CUBE -COLLIDE_MARGIN)
  {
      is_jumping = false;
    youWon = true; camera_y = 0.1; CAMERA_SINK *= -2; rot_y = -M_PI_2;
    return true;
  }
 }
 else {
     is_jumping = false;
    youWon = true; camera_y = 0.1; CAMERA_SINK *= -2; rot_y = -M_PI_2;
    return true;
 }

 //Maze proper
 for (y = 0; y < YSIZE * 2; ++y)
 {
     for (x = 0; x < XSIZE; ++x)
     {
         if (y%2 == 0 && WALL_TEXTURES[y][x]) {
             if (y_at <= MAZE_EXTREME_TOP + (y / 2) * FULL_CUBE + COLLIDE_MARGIN &&
                 y_at >= MAZE_EXTREME_TOP + (y / 2) * FULL_CUBE - COLLIDE_MARGIN&&
                 x_at >= MAZE_EXTREME_LEFT + x * FULL_CUBE - COLLIDE_MARGIN &&
                 x_at <= MAZE_EXTREME_LEFT + (x+1) * FULL_CUBE + COLLIDE_MARGIN) {
                 return 1;
             }             
         }
         else if (y%2 == 1 && WALL_TEXTURES[y][x]) {
             if (x_at <= MAZE_EXTREME_LEFT + x * FULL_CUBE + COLLIDE_MARGIN &&
                 x_at >= MAZE_EXTREME_LEFT + x * FULL_CUBE - COLLIDE_MARGIN&&
                 y_at >= MAZE_EXTREME_TOP + (y/2) * FULL_CUBE - COLLIDE_MARGIN &&
                 y_at <= MAZE_EXTREME_TOP + (y/2 + 1) * FULL_CUBE + COLLIDE_MARGIN) {
                 return 1;
             }
         }
     }
 }
 return 0;
}

void resetGame() {
    x_at = X_INIT;
    y_at = Y_INIT;
    gameOver = youWon = false;
    
    startTime = clock(), endTime = clock() + 185000;
    sec, mn = 0, hour = 0;
       
    rot_x = M_PI_2;
    xin = 0, yin = 0;
    camera_y = START_CAMERA_Y;
    CAMERA_SINK *= -0.5;
    rot_y = -M_PI_2;
    victorySoundPlayed = false;
    
    PlaySound(TEXT("resources/wav/background_music.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}


void move_relative(GLfloat forward_amt, GLfloat strafe_amt)
{
    GLfloat orig_x = x_at;
    GLfloat orig_y = y_at;

    // Forward/Backward vectors
    GLfloat fx = cos(rot_x) * forward_amt;
    GLfloat fy = sin(rot_x) * forward_amt;

    // Strafe Right/Left vectors (A = Left, D = Right)
    GLfloat sx = -sin(rot_x) * strafe_amt;
    GLfloat sy = cos(rot_x) * strafe_amt;

    x_at += fx + sx;
    y_at += fy + sy;

    if (collide())
    {
        x_at -= BOUNCEBACK * (fx + sx);
        y_at -= BOUNCEBACK * (fy + sy);
    }
    if (collide())
    {
        x_at = orig_x;
        y_at = orig_y;
    }
}

void update_movement()
{
    // Calculate Delta Time
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    float deltaTime = (timeSinceStart - oldTimeSinceStart) / 1000.0f;
    oldTimeSinceStart = timeSinceStart;

    if (deltaTime > 0.1f) deltaTime = 0.1f;

    // Handle Jump Physics if jumping
    if (is_jumping && !gameOver && !youWon) {
        // 1. Apply gravity to vertical speed over time
        vertical_velocity += GRAVITY * deltaTime;

        // 2. Move the camera height based on current vertical speed
        camera_y += vertical_velocity * deltaTime;
        // 3. Check if we hit or passed below the floor level
        if (camera_y <= FLOOR_HEIGHT) {
            camera_y = FLOOR_HEIGHT; // Snap safely to floor
            vertical_velocity = 0.0f;
            is_jumping = false;      // Ready to jump again
        }
    }

    // --- Keep your existing horizontal walking / strafing code below ---
    float current_walk_speed = WALK_KEY_SENSE * 60.0f * deltaTime;
    float current_reverse_speed = WALK_KEY_REVERSE_SENSE * 60.0f * deltaTime;
    float current_turn_speed = ROTATE_KEY_SENSE * 60.0f * deltaTime;

    float forward_amt = 0.0f;
    float strafe_amt = 0.0f;

    if (keys['w'] || keys['W'] || specialKeys[GLUT_KEY_UP])    forward_amt += current_walk_speed;
    if (keys['s'] || keys['S'] || specialKeys[GLUT_KEY_DOWN])  forward_amt -= current_reverse_speed;
    if (keys['a'] || keys['A'])                                strafe_amt -= current_walk_speed;
    if (keys['d'] || keys['D'])                                strafe_amt += current_walk_speed;

    if (forward_amt != 0.0f || strafe_amt != 0.0f) {
        move_relative(forward_amt, strafe_amt);
    }

    if (!gameOver && !youWon) {
        if (specialKeys[GLUT_KEY_RIGHT]) rot_x += current_turn_speed;
        if (specialKeys[GLUT_KEY_LEFT])  rot_x -= current_turn_speed;
    }
}

void drawscene()
{  
 static bool init=0;
 static GLuint textureNums = TEXTURE_PATHS.size();
 static GLuint* walls = new GLuint[textureNums]; /*Textures for the cube*/
 static GLuint haze_tex; /*Texture for the sky*/
 static GLuint sky_tex; /*Texture for the sky*/
 static GLuint floor_tex; /*Texture for the sky*/
 static GLuint gover_tex; /*Texture for the sky*/
 static GLuint ywon_tex; /*Texture for the sky*/
 static GLuint grnd_tex; /*Texture for the sky*/
 
 if(!init)
 { 
  init=1;
  for(int i=0;i<textureNums;i++)
    walls[i] = maketex(&TEXTURE_PATHS[i][0], TEXTURE_SIZE, TEXTURE_SIZE);
  haze_tex = maketex(HAZE_FILE, SKY_SIZE_X, SKY_SIZE_Y);
  sky_tex = maketex(SKY_FILE, 612, 408);
  floor_tex = maketex(GRASS_FILE, 375, 375);
  gover_tex = maketex(GOVER_FILE, 800, 512);
  ywon_tex = maketex(YWON_FILE, 800, 512);
  grnd_tex=walls[textureNums-1];
  CONTROLLER_PLAY = min(windowwidth(), windowheight())/2.3f;
  x_at = X_INIT;
  y_at = Y_INIT;
 }

 if(abs(xin)>40 && abs(yin) > 40 && abs(xin - windowwidth())>40&&abs(yin - windowheight()) > 40)
 {
    if (yin<CONTROLLER_PLAY || yin>(windowheight() - CONTROLLER_PLAY))
        rot_y -= (yin - (windowheight() / 2.0f)) * ROTATE_MOUSE_SENSE* windowwidth() / windowheight();

    if (rot_y < 0) rot_y = max(rot_y, -M_PI_2);
    else rot_y = min(rot_y, M_PI_2);
  
    if(xin<CONTROLLER_PLAY || xin>(windowwidth()-CONTROLLER_PLAY))
        rot_x+=(xin-(windowwidth()/2.0f))*ROTATE_MOUSE_SENSE;
 }
 
 glLoadIdentity(); // Make sure we're no longer rotated.

 update_movement(); // Calculate smooth positions right before drawing

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


 string timeStr = timeLeft([](int s, int m, int h, int n) {
        sec=s; mn=m; hour=h;
        if (n < 0) {
        gameOver = true;camera_y = 0.1;CAMERA_SINK *= -2; rot_y = rot_x = -M_PI_2;
    }
 }, endTime);

 gluLookAt(x_at,camera_y,y_at,x_at+cos(rot_x),camera_y+sin(rot_y), y_at + sin(rot_x), 0.0, 1.0, 0.0);

 if (camera_y > 0.0 && !is_jumping) camera_y -= CAMERA_SINK;

 print_maze(walls);// Draw the walls
 floor(grnd_tex); //Draw floor
 
 if (!gameOver && !youWon) {
     sky(haze_tex, sky_tex, floor_tex);
     draw_HUD(drawText("Time Remaining: " + timeStr));
 }
 else {
     if (camera_y > 150) resetGame();
     if (gameOver) {
         sky(gover_tex, sky_tex, floor_tex); //Draw sky
         draw_HUD(drawText("Time's Up!"));
     }
     else if (youWon) {
         sky(ywon_tex, sky_tex, floor_tex); //Draw sky
         draw_HUD(drawText("You Won!"));

         // --- TRIGGER VICTORY AUDIO HERE ---
         if (!victorySoundPlayed) {
             // SND_ASYNC plays it smoothly in the background
             // We omit SND_LOOP so it plays exactly once
             PlaySound(TEXT("resources/wav/victory_music.wav"), NULL, SND_FILENAME | SND_ASYNC);

             victorySoundPlayed = true; // Lock it so it doesn't loop aggressively
         }
     }
 }
 draw_HUD(draw_ortho_compass(rot_x));
 const bool stimulusActive = flickerEnabled && !gameOver && !youWon;
 draw_ssvep_targets(stimulusFrame, stimulusActive, displayRefreshHz);

 glutSwapBuffers();
 if (stimulusActive) ++stimulusFrame;
 else stimulusFrame = 0;
}


void mouse(int x, int y)
{
    int cx = windowwidth() / 2;
    int cy = windowheight() / 2;

    if (x == cx && y == cy) return;

    int dx = x - cx;
    int dy = y - cy;

    // Horizontal rotation (Yaw)
    rot_x += dx * MOUSE_SENSITIVITY;

    // Vertical rotation (Pitch)
    rot_y -= dy * MOUSE_SENSITIVITY; // Subtracting ensures looking up moves camera up

    // Clamp vertical look so you can't flip upside down (approx 85 degrees)
    if (rot_y > 1.48f)  rot_y = 1.48f;
    if (rot_y < -1.48f) rot_y = -1.48f;

    glutWarpPointer(cx, cy);
}

// --- NORMAL KEYS ---
void keypress(unsigned char key, int x, int y)
{
    if (key == ESCAPE) exit(0);

    // Trigger jump if Spacebar is pressed and player is currently on the ground
    if (key == ' ' && !is_jumping && !gameOver && !youWon) {
        vertical_velocity = JUMP_VELOCITY;
        is_jumping = true;
    }

    keys[key] = true;
}

void keypress_up(unsigned char key, int x, int y)
{
    keys[key] = false; // Key was released
}

// --- ARROW / SPECIAL KEYS ---
void arrows(int key, int x, int y)
{
    if (key == GLUT_KEY_F1 && !specialKeys[GLUT_KEY_F1]) {
        flickerEnabled = !flickerEnabled;
        stimulusFrame = 0;
    }
    if (key >= 0 && key < 256) specialKeys[key] = true;
}

void arrows_up(int key, int x, int y)
{
    if (key >= 0 && key < 256) specialKeys[key] = false;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(windowwidth(), windowheight());
    glutInitWindowPosition(WINDOW_STARTX, WINDOW_STARTY);

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    // FIXED: You must create the window BEFORE registering callbacks or setting up GL
    int window = glutCreateWindow("SSVEP Navigation UI");
    const DisplayTiming timing = initializeDisplayTiming();
    displayRefreshHz = timing.refreshHz;
    flickerEnabled = timing.vsyncEnabled;
    glutIgnoreKeyRepeat(1);

    glutDisplayFunc(drawscene);
    glutIdleFunc(drawscene); // Keeps the loop executing continuously
    glutReshapeFunc(resizer);

    // Smooth Input Registrations
    glutKeyboardFunc(keypress);
    glutKeyboardUpFunc(keypress_up);
    glutSpecialFunc(arrows);
    glutSpecialUpFunc(arrows_up);

    glutPassiveMotionFunc(mouse);
    glutSetCursor(GLUT_CURSOR_NONE);

    initgl(windowwidth(), windowheight());
    glDisable(GL_ALPHA_TEST);

    // Prime the clock right before entering loop
    oldTimeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glutWarpPointer(windowwidth() / 2, windowheight() / 2);

    parseMaze(XSIZE, YSIZE, "maze1.txt");


    // --- START BACKGROUND MUSIC HERE ---
    // SND_ASYNC = plays on background thread so game doesn't freeze
    // SND_LOOP  = loops the track indefinitely
    // SND_FILENAME = tells it to look for an external file path string
    PlaySound(TEXT("resources/wav/background_music.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);

    glutMainLoop();

    return 0;
}

