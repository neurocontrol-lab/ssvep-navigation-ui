#include"Constants.h"

extern GLint CONTROLLER_PLAY = 250;
extern GLint WINDOW_STARTX = 20;
extern GLint WINDOW_STARTY = 20;
extern GLint ESCAPE = 27; /* ASCII code for the escape key. */
extern GLint TEXTURE_SIZE = 256;
extern GLint SKY_SIZE_X = 956;
extern GLint SKY_SIZE_Y = 754;
extern GLint BMP_HEADER_SIZE = 54; //Assumed 24-bit depth
extern GLint WINDOW_MARGIN = 100;
extern GLfloat MAZE_EXTREME_LEFT = -20.0f;
extern GLfloat MAZE_EXTREME_TOP = -40.0f;
extern GLfloat HALF_CUBE = 1.25f;
extern GLfloat FULL_CUBE = HALF_CUBE + HALF_CUBE;
extern GLfloat WALL_HT = FULL_CUBE;
extern GLfloat START_X_AT = -10.0f - HALF_CUBE;
extern GLfloat START_Y_AT = 0.0f - HALF_CUBE;
extern GLfloat START_CAMERA_Y = 100.0f;
extern GLfloat CAMERA_SINK = 0.035f;
extern GLfloat VIEW_FIELD = 45.0f;
extern GLfloat NEAR_Z = 0.1f;
extern GLfloat FAR_Z = 1000.0f;
extern GLfloat SKY_DISTANCE = 250.0f;
extern GLfloat LEFTMOST_CUBE_CENTER = MAZE_EXTREME_LEFT + HALF_CUBE;
extern GLfloat COLLIDE_MARGIN = 0.5;  //To keep from looking inside the cubes
extern GLfloat ROTATE_MOUSE_SENSE = 0.000006f;
extern GLfloat ROTATE_KEY_SENSE = 0.08f;
extern GLfloat WALK_MOUSE_SENSE = 0.00012f;
extern GLfloat WALK_KEY_SENSE = 0.5;
extern GLfloat WALK_MOUSE_REVERSE_SENSE = 0.00008f; //Slower when backpedaling
extern GLfloat WALK_KEY_REVERSE_SENSE = 0.5f;
extern GLfloat BOUNCEBACK = 5.0f; //1.0f means none (just reverse collision)
extern GLfloat SKY_SCALE = 6.0f;

//Maze compile-time parameters
extern GLfloat X_INIT = LEFTMOST_CUBE_CENTER + (8 / 2) * FULL_CUBE;
extern GLfloat Y_INIT = MAZE_EXTREME_TOP + (8 / 2) * FULL_CUBE + HALF_CUBE;
extern GLint XSIZE = 8;
extern GLint YSIZE = 8;

extern std::vector<std::vector<int>> WALL_TEXTURES = {};
extern std::vector<std::string> TEXTURE_PATHS = {};

//Logic constants for map contents
extern GLint SOLUTION_PATH = 2;
extern GLint FALSE_PATH = 1;
extern GLint NO_PATH = 0;

// Provisional calibration settings. Require photodiode and EEG validation.
const unsigned SSVEP_FORWARD_PERIOD_FRAMES = 7;
const unsigned SSVEP_BACKWARD_PERIOD_FRAMES = 6;
const unsigned SSVEP_TURN_LEFT_PERIOD_FRAMES = 5;
const unsigned SSVEP_TURN_RIGHT_PERIOD_FRAMES = 4;
const SsvepTargetConfig SSVEP_TARGETS[4] = {
    {0.50f, 0.88f, "FORWARD [Up / W]", SSVEP_FORWARD_PERIOD_FRAMES},
    {0.50f, 0.12f, "BACKWARD [Down / S]", SSVEP_BACKWARD_PERIOD_FRAMES},
    {0.12f, 0.50f, "TURN LEFT [Left]", SSVEP_TURN_LEFT_PERIOD_FRAMES},
    {0.88f, 0.50f, "TURN RIGHT [Right]", SSVEP_TURN_RIGHT_PERIOD_FRAMES}
};
// Target size scales to the viewport relative to this reference resolution.
const float SSVEP_REFERENCE_WIDTH_PX = 1920.0f;
const float SSVEP_REFERENCE_HEIGHT_PX = 1080.0f;
const float SSVEP_TARGET_SIZE_PX = 140.0f;
// Fixed-pixel spacing matches GLUT's non-scaling bitmap font.
const float SSVEP_PANEL_PADDING_PX = 8.0f;
const float SSVEP_BORDER_WIDTH_PX = 2.0f;
const float SSVEP_LABEL_OFFSET_PX = 16.0f;
const float SSVEP_TEXT_LINE_HEIGHT_PX = 15.0f;
const float SSVEP_FOOTER_BASELINE_PX = 12.0f;
