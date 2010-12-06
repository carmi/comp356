/** Maze solving 3D game.
 *
 *  @author Evan Carmi
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "geom356.h"
#include "list356.h"
#include "debug.h"
#include "maze.h"

// Define Window Defaults.
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_XORIG 0
#define DEFAULT_WINDOW_YORIG 0
#define WINDOW_TITLE "3D Interactive Maze Game"

// Window data.
int win_width;
int win_height;

// Up direction.
vector3_t up_dir = {0.0f, 0.0f, 1.0f};

// View-volume specification in camera frame basis.
float view_plane_near = 0.05f;
float view_plane_far = 40.0f;

// Define width and height of maze cell
#define MAZE_CELL_LENGTH 1.0f

// The maze itself with beginning and end cells.
maze_t* maze;
cell_t* maze_start;
cell_t* maze_end;

// Viewing data - x, y, and z coordinates of eye position. This represents
// where the "player" is in the maze.
float pos_x, pos_y, pos_z;

// Define the offset of height that player should look at. 0 is straight
// forwards, negative values are looking down at the floor.
#define LOOK_AT_HEIGHT_OFFSET -0.10

// The look_at vector, the offset from pos_* to where the "player" is looking.
float look_at_x, look_at_y, look_at_z;

// Heading - a scale [0, 360) of degrees that indicate the direction you are
// facing in the maze. heading of 0 is North, and heading increases clockwise.
// So a heading of 90 is East, 270 is West.
float heading;

// Define how big each "step" is on a key-press event
#define POS_DIST_INCR .2f

// Define how much the heading changes on left, right key-press events.
#define HEADING_INCR 5.0

// Define animation of bird's eye perspective. Separate animation into
// ANIMATE_STEPS steps.
#define ANIMATE_STEPS 10.0

// Duration of animation (in nanoseconds): used to determine time to sleep
// between animate steps.
#define ANIMATE_DURATION 300000000

// Height above maze in bird eye view.
#define BIRD_EYE_HEIGHT 8

// Height above maze in normal view.
#define EYE_HEIGHT 0.75

// Bird eye state animation variables.
// NOTE: animation in GL cannot be down with a recursive function because the
// screen will not redraw until all GL functions return. So in handle_key() we
// can't simply call a recursive function that does one step of the animation
// and then calls itself again. We need global state variables to keep track of
// the progress of the animation.
enum bird_eye_state_t {DOWN, UPWARDS, UP, DOWNWARDS};
enum bird_eye_state_t bird_eye_state;
int bird_eye_depth;
float bird_eye_height_step;

// Application data
// If "player" has reached the end cell of the maze.
bool end_reached = false;

// If a move returned -1 because of a wall, print visual feedback. hit_wall
// keeps track of when a wall was hit.
bool hit_wall = false;

// Draw breadcrumbs by keeping track of visited cells.
void* visited_cells;

// Sleep times; globals so they only need be calculated once.
struct timespec interval;
struct timespec remaining;

// Define length (canonical object scaled along x-axis) of wall.
#define WALL_LENGTH_SCALE 1.25f

// Define width (canonical object scaled along y-axis) of wall.
#define WALL_WIDTH_SCALE 1.0f

// Define height (canonical object scaled along z-axis) of wall.
#define WALL_HEIGHT_SCALE 1.0f

// Materials and lights.  Note that colors are RGBA colors.  OpenGL lights
// have diffuse, specular, and ambient components; we'll default to setting
// the first two equal to each other and the OpenGL default for ambient
// (black).  And the position is in homogeneous coordinates, so a w-value
// of 0 means "infinitely far away."
typedef struct _material_t {
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat phong_exp;
} material_t;

typedef struct _light_t {
    GLfloat position[4];
    GLfloat color[4];
} light_t;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0};

light_t far_light = {
    {20.0, 10.0, 0.0, 1.0},
    {0.75, 0.75, 0.75, 1.0}
};

light_t red_light = {
    {20.0, 10.0, 0.0, 1.0},
    {0.9, 0.25, 0.25, 1.0}
};

material_t gold = {
    {.10f, .084f, 0.0f, 1.0f},
    {1.0f, .84f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

material_t red_plastic = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

material_t green_plastic = {
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

material_t t_plastic = {
    {0.75f, 0.0f, 0.75f, 1.0f},
    {0.75f, 0.0f, 0.75f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
};

// Function Declarations

// Callbacks.
void handle_display(void);
void handle_resize(int, int);
void handle_key(unsigned char, int, int);
void handle_special_key(int, int, int);

// Application functions.
void init();
void init_maze(int maze_height, int maze_width);
int move(float new_x, float new_y);

// GL functions.
void init_gl();
void set_lights();
void set_camera();
void set_look_at();

// Canonical structure creation functions.
void draw_axes();
void draw_square(material_t material);
void draw_marker(int row, int col, float scale, material_t material);
void draw_rect();
void draw_wall(float row, float col, unsigned char direction);
void draw_maze();

int main(int argc, char **argv) {
    //debug("main()");

    // Initialize the drawing window.
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    glutInitWindowPosition(DEFAULT_WINDOW_XORIG, DEFAULT_WINDOW_YORIG);
    glutInit(&argc, argv);

    // Create the main window.
    debug("Creating window");
    glutCreateWindow(WINDOW_TITLE);

    debug("Setting callbacks");
    glutReshapeFunc(handle_resize);
    glutKeyboardFunc(handle_key);
    glutSpecialFunc(handle_special_key);
    glutDisplayFunc(handle_display);

    // Initialize the maze.
    int maze_width = atoi(argv[1]);
    int maze_height = atoi(argv[2]);
    init_maze(maze_height, maze_width);

    // Initialize application state

    init();

    // Initialize GL.
    init_gl();

    // Enter the main event loop.
    debug("Main loop");
    glutMainLoop();

    return EXIT_SUCCESS;
}

/**
 * Create the maze and start and end cells.
 * 
 * @param height - height of maze.
 * @param width - width of maze.
 */
void init_maze(int maze_height, int maze_width) {
    debug("init_maze()");

    maze = make_maze(maze_height, maze_width, time(NULL));
    maze_start = get_start(maze);
    maze_end = get_end(maze);
}

/**
 * Initialize maze and application state.
 */
void init() {
    debug("init()");

    // Setup sleep timing intervals
    interval.tv_sec = 0;
    interval.tv_nsec = ANIMATE_DURATION/ANIMATE_STEPS;

    // Setup player position and viewpoint to be in center of start cell.
    pos_x = maze_start->c + MAZE_CELL_LENGTH/2.0f;
    pos_y = maze_start->r + MAZE_CELL_LENGTH/2.0f;
    pos_z = EYE_HEIGHT;
    set_look_at();

    // Start maze game at bird_eye view heading downwards.
    bird_eye_depth = ANIMATE_STEPS;
    bird_eye_state = UPWARDS;

    // Calculate once the height step for bird eye animation.
    bird_eye_height_step = (float) (BIRD_EYE_HEIGHT/ANIMATE_STEPS);

    // Set the heading.
    heading = 0.0f;

    // Create list to keep track of visited cells
    visited_cells = make_list();

    // Set the lights.
    set_lights(far_light);
}

/**
 * Initialize OpenGL. Set our world frame to be the width and height of the maze.
 */
void init_gl() {
    debug("init_gl");
    // GL initialization.
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
    
    // Background color.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * Change value of bird_eye_state variable. Calling this method simply sets the
 * value of bird_eye_state to the next value in the list of possibilities or
 * cycles the state. The cycle should look like:
 * DOWN --> UPWARDS --> UP --> DOWNWARDS --> DOWN
 */
void bird_eye_toggle() {
    debug("bird_eye_toggle");

    // Use fact that enumerated types are ordered integers.
    if (bird_eye_state == DOWNWARDS) {
        bird_eye_state = DOWN;
    } else {
        bird_eye_state += 1;
    }
}

/** 
 * Set the camera transformation for the bird's eye view.  We look at the
 * position of player given by pos_[x|y|z] from bird eye at {pos_x, pos_y,
 * pos_z + some height} where height depends on which step or depth of
 * bird_eye animation we are at. Our up direction is the direction the player
 * is facing (the look_at vector in set_camera()).
 *
 * Animation is defined by ANIMATE_* variables.
 *
 * This method mimics a recursive function. It either decrements the global
 * bird_eye_depth variable and calls itself again, or returns (base case) if
 * depth == 0.
 */
void set_bird_eye() {
    debug("set_bird_eye");
    if (bird_eye_depth == 0) {
        // Base case
        if (bird_eye_state == DOWNWARDS) set_camera();
        bird_eye_toggle();
        return;
    }
    else {
        // Set the model-view (i.e., camera) transform.
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // TODO, do this manually instead of with gluLookAt

        float height;
        if (bird_eye_state == UPWARDS) {
            // Need the +1 because on first call bird_eye_depth ==
            // ANIMATE_STEPS but we want the bird_eye_height_step to be
            // multiplied by 1, not 0.
            height = bird_eye_height_step*(ANIMATE_STEPS - bird_eye_depth + 1);
        } else if (bird_eye_state == DOWNWARDS) {
            height = bird_eye_height_step*(bird_eye_depth);
        } else assert(0);

        point3_t bird_eye = {pos_x, pos_y, (pos_z + height) };

        // Look at is x, and y values of normal look_at_* vector, but z should
        // always be 0 because the bird eye is looking straight down on the
        // maze.
        point3_t bird_look_at = {look_at_x, look_at_y, 0.0f};

        debug("bird_eye vector = (%f, %f, %f)", bird_eye.x, bird_eye.y,
                                bird_eye.z);
        debug("Look at vector = (%f, %f, %f)", pos_x, pos_y, pos_z);
        debug("up vector(look_at_*) = (%f, %f, %f)", bird_look_at.x,
                                bird_look_at.y, bird_look_at.z);

        // Up vector is same as look_at_* offset. Thus bird_eye view "up" is
        // "forwards" in normal maze.
        gluLookAt(bird_eye.x, bird_eye.y, bird_eye.z,
                  pos_x, pos_y, pos_z,
                  bird_look_at.x, bird_look_at.y, bird_look_at.z);

        bird_eye_depth -= 1;
        nanosleep(&interval, &remaining);
        glutPostRedisplay();
    }
}


/** Set the camera transformation.  Viewpoint is given by the eye
 *  coordinates; we look at the origin with up-direction along the
 *  +z axis.
 */
void set_camera() {
    debug("set_camera()");

    // Set the model-view (i.e., camera) transform.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //TODO, do this manually instead of with gluLookAt

    gluLookAt(pos_x, pos_y, pos_z,
            pos_x + look_at_x, pos_y + look_at_y, pos_z + look_at_z,
            up_dir.x, up_dir.y, up_dir.z);
}

/** Set the projection and viewport transformations.  We use perspective
 *  projection and always match the aspect ratio of the screen window
 *  with vertical field-of-view 60 degrees and always map to the entire
 *  screen window.
 */
void set_projection_viewport() {
    debug("set_projection_viewport()");

    // Set perspective projection transform.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)win_width/win_height, view_plane_near, 
            view_plane_far);

    // Set the viewport transform.
    glViewport(0, 0, win_width, win_height);
}

/** Set the light colors.  Since the position of the light
 *  is subject to the current model-view transform, and we have
 *  specified the light position in world-frame coordinates,
 *  we want to set the light position after setting the camera
 *  transformation; since the camera transformation may change in response
 *  to keyboard events, we ensure this by setting the light position
 *  in the display callback.
 *
 *  It is also easy to "attach" a light to the viewer.  In that case,
 *  just specify the light position in the camera frame and make sure
 *  to set its position while the camera transformation is the identity!
 */
void set_lights(light_t* light_source) {
    debug("set_lights()");

    light_t* light = &light_source;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light->color);
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light->color);
}

/** Handle a resize event by recording the new width and height.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {
    debug("handle_resize(%d, %d)\n", width, height);

    win_width = width;
    win_height = height;

    set_projection_viewport();

    glutPostRedisplay();
}

/**
 * Update the global look_at vector look_at_x, look_at_y, look_at_z. This sets
 * the look_at vector by using the heading and current position pos_[x|y|z] of
 * "player". This function should be called after the pos_* or heading values
 * change.
 * Note: this is the offset or difference between the look_at vector and the
 * position vector. Thus in set_camera the look_at vector to be used is pos_*
 * + look_at_*, NOT simply look_at_*.
 */
void set_look_at() {
    //debug("look_at()");
    look_at_x = sin((double) (heading*M_PI/180.0f) );
    look_at_y = cos((double) (heading*M_PI/180.0f) );
    look_at_z = LOOK_AT_HEIGHT_OFFSET;
    debug("look_at = (%f, %f, %f)", look_at_x, look_at_y, look_at_z);
}

/**
 * Move the "player" as defined by pos_* coordinates to new position, unless
 * new position is inside a wall.
 *
 * @param new_x - the x coordinate of the new desired position
 * @param new_y - the y coordinate of the new desired position
 * Return 0 if move was successful (there were no walls in the way), otherwise
 * return -1.
 * Prerequisite: POS_DIST_INCR is less than width of walls.
 */
int move(float new_x, float new_y) {
    debug("move(x=%f, y=%f)", new_x, new_y);
    // Check if new position will be inside a wall. Assuming POS_DIST_INCR is
    // smaller than width of walls.

    int new_x_floor = (int) new_x;
    int new_y_floor = (int) new_y;

    // Get cell of new position, if inside maze. (Without this we could get a
    // segfault when moving at edges of the maze with a high POS_DIST_INCR).
    if ( (new_x_floor < 0) ||
         (new_x_floor >= get_ncols(maze)) ||
         (new_y_floor < 0) ||
         (new_y_floor >= get_nrows(maze)) ) {
        debug("Segfault protection prevented a move");
        return -1;
    }
    cell_t* cell = get_cell(maze, new_y_floor, new_x_floor);

    // Get decimal of position; if it is near the edge of the cell, see if
    // there is a wall on that direction.
    float dec_x = (new_x - new_x_floor);
    float dec_y = (new_y - new_y_floor);
    
    // Walls go into cells (0.25 * WALL_WIDTH_SCALE) factor on each
    // side. So on one side of cell it's half that.
    float wall_width = (WALL_WIDTH_SCALE*0.25f)/2.0f;

    // If dec_x or dec_y are within the top or bottom range of wall_width, then
    // the position is inside a wall.
    
    // TODO: cleanup and test corners more.
    if (dec_x <= wall_width) {
        debug("west");
        if (has_wall(maze, cell, WEST)) {
            debug("not moving");
            return -1;
        }
    } else if (dec_x >= MAZE_CELL_LENGTH - wall_width) {
        debug("east");
        if (has_wall(maze, cell, EAST)) {
            debug("not moving");
            return -1;
        }
    } else if (dec_y <= wall_width) {
        debug("south");
        if (has_wall(maze, cell, SOUTH)) {
            debug("not moving");
            return -1;
        }
    } else if (dec_y >= MAZE_CELL_LENGTH - wall_width) {
        debug("north");
        if (has_wall(maze, cell, NORTH)) {
            debug("not moving");
            return -1;
        }
    }
    // No walls were encountered, move.
    debug("moving");
    pos_x = new_x;
    pos_y = new_y;
    add_visited_cell(cell);

    // Check if "player" has reached end cell. If so, change lights & display
    // text.
    if ((maze_end->c  == (int) pos_x) && (maze_end->r == (int) pos_y)) {
        set_lights(&red_light);
        end_reached = true;
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
    }
    return 0;
}

/**
 * Add a cell to the list of visited cells if it does not already exist there.
 * @param cell - the cell to add to the list of visited cells visited_cells.
 */
void add_visited_cell(cell_t* cell) {
    // Add cell if not in list.
    if (!lst_contains(visited_cells, cell, cell_cmp)) {
        lst_add(visited_cells, cell);
    }
}

/** Draw the coordinate axes as line segments from -100 to +100 along
 *  the corresponding axis.
 */
void draw_axes() {
    // Disable lighting so that we can set the color with glColor*.
    // Remember to enable it when done!
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);
    for (int j=0; j<get_nrows(maze) + 1; ++j) {
        glVertex3f(0.0f, (float) j, 0.0f);
        glVertex3f((float) get_ncols(maze), (float) j, 0.0f);
    }
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    // Draw lines parallel to y-axes.
    for (int i=0; i<get_ncols(maze) + 1; ++i) {
        glVertex3f((float) i, 0.0f, 0.0f);
        glVertex3f((float) i, (float) get_nrows(maze), 0.0f);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

/**
 * Draw a square of side-length 2 in the xy-plane centered at the origin. This
 * is used to color the start and end cells of the maze.
 * @param material - the material of the square.
 */
void draw_square(material_t material) {
    //debug("draw_square()");

    // Specify the material for the square.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, material.phong_exp);

    glBegin(GL_QUADS);
    // z=0 plane.
    glNormal3f(0.0, 0.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(-1.0, 1.0, 0.0);
    glEnd();
}

/**
 * Draw a marker square on the bottom floor of the maze by transforming the
 * draw_square object. The new marker square will be centered at cell given in
 * the parameters.
 * @param row - the row of the maze cell to place the marker.
 * @param col - the column of the maze cell to place the marker.
 * @param scale - the value by which to scale the 
 * @param material - the material of the square.
 */
void draw_marker(int row, int col, float scale, material_t material) {
    // Make sure we're talking about the m-v xfrm stack.
    glMatrixMode(GL_MODELVIEW);

    // Push a copy of the current m-v xfrm onto the stack.
    glPushMatrix();

    // Define model transform.
    glTranslatef(col + 0.5f, row + 0.5f, 0.0f);
    glScalef(scale, scale, 1.0f);

    // Draw the square.
    draw_square(material);

    // Undo the model transform.
    glPopMatrix();
}


/**
 * Draw a wall in the maze in the world frame by translating and rotating a
 * draw_square object. A wall is a draw_rect rectangle that sits on the
 * positive x-z plane with length and height 1.25 and width .25.
 * @param row - the row of the maze cell.
 * @param col - the column of the maze cell.
 * @param direction - in which direction to draw the wall. This is an unsigned
 * char defined in maze.h and maze.c.
 */
void draw_wall(float row, float col, unsigned char direction) {
    //debug("draw_wall()");

    // TODO: should this possibly be further up the call sequence?
    // Make sure we're talking about the m-v xfrm stack.
    glMatrixMode(GL_MODELVIEW);

    // Push a copy of the current m-v xfrm onto the stack.
    glPushMatrix();

    // Define the model xfrm.
    // Move walls up onto x-y plane and corner at (0,0,0).
    glTranslatef(col + 0.5f, row, 0.5f);
    // Rotate wall depending on direction.

    switch (direction) {
        case NORTH:
            //debug("NORTH");
            glTranslatef(0.0f, 1.0f, 0.0f);
            break;
        case EAST:
            //debug("EAST");
            glTranslatef(0.5f, 0.5f, 0.0f);
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            break;
        case SOUTH:
            //debug("SOUTH");
            break;
        case WEST:
            //debug("WEST");
            glTranslatef(-0.5f, 0.5f, 0.0f);
            glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
            break;
        default:
            assert(0);
    }
    // Rotate walls to be parallel perpendicular to x-y plane.
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    // Stretch walls so the overlap cleanly.
    glScalef(WALL_LENGTH_SCALE, WALL_WIDTH_SCALE, WALL_HEIGHT_SCALE);

    // Draw the wall.
    draw_rect();

    // Undo the model xfrm.
    glPopMatrix();
}

/**
 * Draw a rectangular solid length 1, width .25, height 1, along the x-axis
 * centered at the origin. This object should be used create the floor and
 * walls of the maze.
 */
void draw_rect() {
    //debug("draw_rect()");

    // Specify the material for the wall.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gold.diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gold.specular);
    glMaterialf(GL_FRONT, GL_SHININESS, gold.phong_exp);

    glBegin(GL_QUADS);
    // z=.125 plane.
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, 0.125);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, 0.125);

    // z=-.125 plane.
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(0.5, -0.5, -0.125);

    // x=0.5 plane.
    glNormal3f(1.0, 0.0, 0.0);
    glVertex3f(0.5, -0.5, 0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(0.5, 0.5, 0.125);

    // x=-0.5 plane.
    glNormal3f(-1.0, 0.0, 0.0);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(-0.5, 0.5, -0.125);
    glVertex3f(-0.5, -0.5, -0.125);

    // y=0.5 plane.
    glNormal3f(0.0, 1.0, 0.0);
    glVertex3f(-0.5, 0.5, 0.125);
    glVertex3f(0.5, 0.5, 0.125);
    glVertex3f(0.5, 0.5, -0.125);
    glVertex3f(-0.5, 0.5, -0.125);

    // y=-0.5 plane.
    glNormal3f(0.0, -1.0, 0.0);
    glVertex3f(-0.5, -0.5, 0.125);
    glVertex3f(-0.5, -0.5, -0.125);
    glVertex3f(0.5, -0.5, -0.125);
    glVertex3f(0.5, -0.5, 0.125);
    glEnd();
}

/**
 * Draw the maze by transforming draw_rect() and draw_square() objects. The
 * maze will be drawn on the x-y plane. 
 */
void draw_maze() {
    //debug("draw_maze()");

    int maze_width = get_ncols(maze);
    int maze_height = get_nrows(maze);

    // Draw breadcrumbs for all cells in visited_cells.
    list356_itr_t* itr = lst_iterator(visited_cells);
    while (lst_has_next(itr)) {
        cell_t* cell = lst_next(itr);
        draw_marker(cell->r, cell->c, 0.15f, blue_plastic);
    }


    // Draw squares on start cell, end cell, current cell.
    draw_marker(maze_start->r, maze_start->c, 0.25f, green_plastic);
    draw_marker(maze_end->r, maze_end->c, 0.25f, red_plastic);
    draw_marker((int) pos_y, (int) pos_x, 0.25f, t_plastic);

    
    // Draw the walls.  First draw the east and south exterior walls, then
    // draw any north or west walls of each cell.

    for (float i=0.0f; i<maze_width; ++i) {
        for (float j=0.0f; j<maze_height; ++j) {
            // Prim's algorithm will always leave a wall segment touching the
            // North-East corner of the cell. Knowing this, we draw this part
            // first, and the rest of our walls than do not have to overlap at
            // all.
            // Draw center box.
            //draw_center(j, i);

            //Draw south wall if height==0
            if (j == 0.0f) {
                draw_wall(j, i, SOUTH);
            }
            // Draw west wall if width==0
            if (i == 0.0f) {
                draw_wall(j, i, WEST);
            }
            if (has_wall(maze, get_cell(maze, (int) j, (int) i), NORTH)) {
                draw_wall(j, i, NORTH);
            }
            if (has_wall(maze, get_cell(maze, (int) j, (int) i), EAST)) {
                draw_wall(j, i, EAST);
            }
        }
    }
}

/** Draw a string on the screen.  The string will be drawn in the current
 *  color and at the current raster position.
 */
void draw_string(char* s, void *font) {
    for (char *c=s; *c!='\0'; ++c) {
        glutBitmapCharacter(font, *c);
    }
}

/** Print the position (distance, theta, and phi) on the screen.
 */
void print_position() {
    glColor3f(0.0f, 0.0f, 0.0f);

    glWindowPos2s(10, 30);
    char* s;
    asprintf(&s, "Camera position = (%f, %f, %f)", pos_x, pos_y, pos_z);
    draw_string(s, GLUT_BITMAP_TIMES_ROMAN_10);
    free(s);

    glWindowPos2s(10, 10);
    asprintf(&s, "Heading = %f; Heading (rads) = %f", heading, heading*M_PI/180.0f);
    draw_string(s, GLUT_BITMAP_TIMES_ROMAN_10);
    free(s);



    // If reached end cell, inform "player"
    if (end_reached) {
        glWindowPos2s(win_width/2 - 100, win_height - 100);
        asprintf(&s, "Congratulations!");
        draw_string(s, GLUT_BITMAP_TIMES_ROMAN_24);
        free(s);
        glWindowPos2s(win_width/2 - 170, win_height - 130);
        asprintf(&s, "You've have reached the end cell.");
        draw_string(s, GLUT_BITMAP_TIMES_ROMAN_24);
        free(s);
    }
    
    // If we just hit a wall, inform "player"
    if (hit_wall) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glWindowPos2s(win_width/2 - 220, win_height - 70);
        asprintf(&s, "You can't move. There's a wall in the way!");
        draw_string(s, GLUT_BITMAP_TIMES_ROMAN_24);
        free(s);
        hit_wall = false;
    }
}

/** Handle a display request by clearing the screen, drawing the axes
 *  and cube, and printing the viewpoint position.
 */
void handle_display() {
    //debug("handle_display()");
    if (!bird_eye_state == DOWN) set_bird_eye();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Note that the lights are sent through the pipeline, and a light's
    // position is modified by the model-view transformation.  By setting the
    // light's position here, we ensure that the light lives in a fixed
    // place in the world.  See set_lights() for more info.
    glLightfv(GL_LIGHT0, GL_POSITION, far_light.position);

    draw_axes();
    
    //draw_rect();
    //draw_square();
    draw_maze();

    print_position();

    glFlush();
    
}

/** Handle keyboard events:
 *  
 *  - SPACE: switch to bird's eye view.
 *  - +, -:  increment/decrement the distance to the origin of the viewpoint.
 *
 *  Redisplays will be requested from every key event.
 *
 *  @param key the key that was pressed.
 *  @param x the mouse x-position when <code>key</code> was pressed.
 *  @param y the mouse y-position when <code>key</code> was pressed.
 */
// I think this has to return before GL does anything else
void handle_key(unsigned char key, int x, int y) {

    switch (key) {
        case ' ':
            debug("handle_key() <space> - bird's eye view %i", bird_eye_state);
            // If not currently animating, start animation.
            if (!((bird_eye_state == UPWARDS) || (bird_eye_state == DOWNWARDS))) {
                bird_eye_depth = ANIMATE_STEPS;
                bird_eye_toggle();
            }
            break;
        case '+':
            debug("handle_key() - increase eye dist.");
            pos_z += POS_DIST_INCR;
            set_look_at();
            set_camera();
            break;
        case '-':
            debug("handle_key() - decrease eye dist.");
            pos_z -= POS_DIST_INCR;
            set_look_at();
            set_camera();
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

/** Handle keyboard events:
 *
 *  - LEFT, RIGHT:  increment/decrement heading.
 *  - UP, DOWN:  move eye position.
 *      Move pos_x += POS_DIST_INCR * sin(heading)
 *      Move pos_y += POS_DIST_INCR * cos(heading)
 *
 *  Redisplays will be requested for every key event.
 *
 *  @param key the key that was pressed.
 *  @param x the mouse x-position when <code>key</code> was pressed.
 *  @param y the mouse y-position when <code>key</code> was pressed.
 */
void handle_special_key(int key, int x, int y) {
    // Disable movement during bird's eye view.
    if (bird_eye_state == DOWN) {
        switch (key) {
            case GLUT_KEY_LEFT:
                heading -= HEADING_INCR;
                if (heading < 0.0f) heading += 360.0f;
                debug("GLUT_KEY_LEFT: heading = %f", heading);
                break;
            case GLUT_KEY_RIGHT:
                debug("GLUT_KEY_RIGHT: heading = %f", heading);
                heading += HEADING_INCR;
                if (heading >= 360.0f) heading -= 360.0f;
                break;
            case GLUT_KEY_UP:
                if (move((pos_x + POS_DIST_INCR*look_at_x), (pos_y + POS_DIST_INCR*look_at_y)) == -1) {
                    hit_wall = true;
                    debug("Can't move, wall in the way");
                }
                debug("GLUT_KEY_UP: pos_x = %f, pos_y = %f", pos_x, pos_y);
                break;
            case GLUT_KEY_DOWN:
                if (move((pos_x - POS_DIST_INCR*look_at_x), (pos_y - POS_DIST_INCR*look_at_y)) == -1) {
                    hit_wall = true;
                    debug("Can't move, wall in the way");
                }
                debug("GLUT_KEY_DOWN: pos_x = %f, pos_y = %f", pos_x, pos_y);
                break;
            default:
                break;
        }
        set_look_at();
        set_camera();
        glutPostRedisplay();
    }
}
