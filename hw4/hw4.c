/** View of several solid cubes (each face a solid color).
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
#include <unistd.h>

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
#include "debug.h"
#include "maze.h"

#define max(a, b) ((a < b ? b : a))

// Define Window Defaults.
#define DEFAULT_WINDOW_WIDTH 800
#define DEFAULT_WINDOW_HEIGHT 600
#define DEFAULT_WINDOW_XORIG 0
#define DEFAULT_WINDOW_YORIG 0
#define WINDOW_TITLE "3D Interactive Maze Game"

// the maze itself
maze_t* maze ;

int frames = 10;
// TODO remove
//int bird_eye_toggle = 0;
bool bird_eye = false;
bool bird_eye_up;
bool bird_eye_down;

// Window data.
int win_width ;
int win_height ;

// Viewing data - x, y, and z coordinates of eye position. The eye represents
// where the "player" is in the maze.
float eye_x, eye_y, eye_z;

// Heading - a scale [0, 360) of degrees that indicate the direction you are
// facing in the maze. heading of 0 is North, and heading increases clockwise.
// So a heading of 90 is East, 270 is West.
float heading;

// Look-at position (world coordinates).
//point3_t look_at = {0.0f, 0.0f, 0.0f} ;
// Up direction.
vector3_t up_dir = {0.0f, 0.0f, 1.0f} ;

#define EYE_DIST_INCR .1f
#define HEADING_INCR 5.0
//
//
// Eye is located distance eye_r from origin; angle eye_theta from +x axis
// in xz-plane; angle eye_phi from xz-plane.
float eye_r, eye_theta, eye_phi ;
#define EYE_THETA_INCR 2.0*M_PI/180 
#define EYE_PHI_INCR 2.0*M_PI/180 

// View-volume specification in camera frame basis.
float view_plane_near = 0.25f ;
float view_plane_far = 40.0f ;

// Bird's eye animation settings
#define ANIMATE_STEPS 20
#define ANIMATE_SLEEP 0.1
#define BIRD_EYE_HEIGHT 20
enum be_state_t {DOWN, UPWARDS, UP, DOWNWARDS};
enum be_state_t bird_eye_state;
int bird_eye_depth;

// Display callback:  draws the maze.
void draw_maze() ;

// Callbacks.
void handle_display(void) ;
void handle_resize(int, int) ;
void handle_key(unsigned char, int, int) ;
void handle_special_key(int, int, int) ;

// Application functions.
void init() ;

// GL initialization.
void init_gl() ;

// Application data.
bool do_print_position ;    // Whether or not to print the position.

// Materials and lights.  Note that colors are RGBA colors.  OpenGL lights
// have diffuse, specular, and ambient components; we'll default to setting
// the first two equal to each other and the OpenGL default for ambient
// (black).  And the position is in homogeneous coordinates, so a w-value
// of 0 means "infinitely far away."
typedef struct _material_t {
    GLfloat ambient[4] ;
    GLfloat diffuse[4] ;
    GLfloat specular[4] ;
    GLfloat phong_exp ;
} material_t ;

typedef struct _light_t {
    GLfloat position[4] ;
    GLfloat color[4] ;
} light_t ;

GLfloat BLACK[4] = {0.0, 0.0, 0.0, 1.0} ;

light_t far_light = {
    {20.0, 10.0, 0.0, 1.0},
    {0.75, 0.75, 0.75, 1.0}
} ;

material_t gold = {
    {.10f, .084f, 0.0f, 1.0f},
    {1.0f, .84f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
} ;

material_t red_plastic = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
} ;

material_t green_plastic = {
    {0.0f, 1.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
} ;

material_t blue_plastic = {
    {0.0f, 0.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f, 1.0f},
    1000.0f
} ;

int main(int argc, char **argv) {
    debug("heading incrment value: %f", HEADING_INCR);

    // Initialize the drawing window.
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH ) ;
    glutInitWindowSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT) ;
    glutInitWindowPosition(DEFAULT_WINDOW_XORIG, DEFAULT_WINDOW_YORIG) ;
    glutInit(&argc, argv) ;

    // Handle command line arguments.
    do_print_position = true ;
    for (int i=0; i<argc; ++i) {
        if (strcmp(argv[i], "--position") == 0) do_print_position = true ;
    }

    // Create the main window.
    debug("Creating window") ;
    glutCreateWindow(WINDOW_TITLE) ;

    debug("Setting callbacks") ;
    glutReshapeFunc(handle_resize) ;
    glutKeyboardFunc(handle_key) ;
    glutSpecialFunc(handle_special_key) ;
    glutDisplayFunc(handle_display) ;

    // Initialize the maze.
    int maze_width = atoi(argv[1]) ;
    int maze_height = atoi(argv[2]) ;
    maze = make_maze(maze_height, maze_width, time(NULL)) ;


    // Initialize GL.
    init_gl() ;

    // Enter the main event loop.
    debug("Main loop") ;
    glutMainLoop() ;

    return EXIT_SUCCESS ;
}

/**
 * Initialize OpenGL. Set our world frame to be the width and height of the maze.
 */
void init_gl() {
    debug("init_gl") ;
    // GL initialization.
    glEnable(GL_DEPTH_TEST) ;
    // glEnable(GL_CULL_FACE) ;
    // glCullFace(GL_BACK) ;
    glEnable(GL_LIGHTING) ;
    glEnable(GL_LIGHT0) ;
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1) ;
    
    // Background color.
    //glClearColor(0.0f, 0.0f, 0.0f, 0.0f) ;
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f) ;

    // TODO: set beginning eye at start cell
    // Viewpoint position.
    eye_x = 0.0f;
    eye_y = 0.0f;
    eye_z = 0.75f;

    bird_eye_state = DOWN;

    // Set the heading.
    heading = 0.0f;

    eye_r = sqrt(48) ;
    eye_theta = M_PI_4 ;
    eye_phi = M_PI_4 ;

    // Set the lights.
    set_lights() ;

    // Set the viewpoint.
    set_camera() ;

}

/**
 * Change value of bird_eye_state variable. Calling this method simply sets the
 * valud of bird_eye_state to the next value in the list of possibilities or
 * cycles the state. The cycle should look like:
 * DOWN --> UPWARDS --> UP --> DOWNWARDS --> DOWN
 */
void bird_eye_toggle() {
    debug("bird_eye_toggle");

    // Use fact enums are ordered integers.
    if (bird_eye_state == DOWNWARDS) {
        bird_eye_state = DOWN;
    } else {
        bird_eye_state += 1;
    }

    /*
    if (bird_eye_toggle == 0) {
        bird_eye_toggle = 1;
        frames = 10;
    }
    else if (bird_eye_toggle == 1) bird_eye_toggle = 2;
    else if (bird_eye_toggle == 2) bird_eye_toggle = 0;

    if (!bird_eye) {
        bird_eye = true;
        bird_eye_up = true;
    }
    else {
        bird_eye = false;
        bird_eye_down = true;
    }

    //frames = 10;
    for (int i = 10; i > 0; i--) {
        set_bird_eye(i);
        glutPostRedisplay() ;
        sleep(1);
    }
    */
}

/** Set the camera transformation for the bird's eye view.  Viewpoint is given
 *  by the eye coordinates; we look at the origin with up-direction along the
 *  +z axis.
 *  animate by making the move in frames steps.
 *
 * This method mimics a recursive function. It either decrements the global
 * bird_eye_depth variable and calls itself again, or returns (base case) if
 * depth == 0.
 *
 */
void set_bird_eye() {
    debug("set_bird_eye");
    if (bird_eye_depth == 0) {
        bird_eye_toggle();
        return;
    }
    else {

        // Set the model-view (i.e., camera) transform.
        glMatrixMode(GL_MODELVIEW) ;
        glLoadIdentity() ;
        // TODO, do this manually instead of with gluLookAt
        // Look at point defined by adding vector from eyepoint in deirection of heading with length 1 unit.

        // Up vector is same as look_at vector in set_camera()
        point3_t up = { eye_x + sin((double) (heading*M_PI/180.0f) ), eye_y + cos((double) (heading*M_PI/180.0f) ), eye_z};
        
        float height_step = (float) (BIRD_EYE_HEIGHT/ANIMATE_STEPS);
        float height;
        if (bird_eye_state == UPWARDS) height = height_step*(ANIMATE_STEPS - bird_eye_depth);
        else if (bird_eye_state == DOWNWARDS) height = height_step*(bird_eye_depth);
        else assert(0); 

        point3_t eye = {eye_x, eye_y, (eye_z + height) };

        debug("Up vector = (%f, %f, %f)", up.x, up.y, up.z) ;
        debug("Eye vector = (%f, %f, %f)", eye.x, eye.y, eye.z) ;
        gluLookAt(eye.x, eye.y, eye.z,
                eye_x, eye_y, eye_z,
                up.x, up.y, up.z) ;

        bird_eye_depth -= 1;
        sleep(ANIMATE_SLEEP);
        glutPostRedisplay() ;
    }
}


/** Set the camera transformation.  Viewpoint is given by the eye
 *  coordinates; we look at the origin with up-direction along the
 *  +z axis.
 */
void set_camera() {
    debug("set_camera()") ;

    // Set the model-view (i.e., camera) transform.
    glMatrixMode(GL_MODELVIEW) ;
    glLoadIdentity() ;
    //TODO, do this manually instead of with gluLookAt
    // Look at point defined by adding vector from eyepoint in deirection of heading with length 1 unit.

    point3_t look_at = {eye_x + sin((double) (heading*M_PI/180.0f) ), eye_y + cos((double) (heading*M_PI/180.0f) ), eye_z};
    debug("Look_at = (%f, %f, %f)", look_at.x, look_at.y, look_at.z) ;
    gluLookAt(eye_x, eye_y, eye_z,
            look_at.x, look_at.y, look_at.z,
            //eye_x + sin((double) heading), eye_y + cos((double) heading), eye_z,
            up_dir.x, up_dir.y, up_dir.z) ;
}

/** Set the projection and viewport transformations.  We use perspective
 *  projection and always match the aspect ratio of the screen window
 *  with vertical field-of-view 60 degrees and always map to the entire
 *  screen window.
 */
void set_projection_viewport() {
    debug("set_projection_viewport()") ;

    // Set perspective projection transform.
    glMatrixMode(GL_PROJECTION) ;
    glLoadIdentity() ;
    gluPerspective(60.0, (GLdouble)win_width/win_height, view_plane_near, 
            view_plane_far) ;

    // Set the viewport transform.
    glViewport(0, 0, win_width, win_height) ;
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
void set_lights() {
    debug("set_lights()") ;

    light_t* light = &far_light ;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light->color) ;
    glLightfv(GL_LIGHT0, GL_AMBIENT, BLACK) ;
    glLightfv(GL_LIGHT0, GL_SPECULAR, light->color) ;
}



/** Handle a resize event by recording the new width and height.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {
    debug("handle_resize(%d, %d)\n", width, height) ;

    win_width = width ;
    win_height = height ;

    set_projection_viewport() ;

    glutPostRedisplay() ;
}

/** Draw the coordinate axes as line segments from -100 to +100 along
 *  the corresponding axis.
 */
void draw_axes() {
    // Disable lighting so that we can set the color with glColor*.
    // Remember to enable it when done!
    glDisable(GL_LIGHTING) ;
    glBegin(GL_LINES) ;
    glColor3f(0.0, 0.0, 1.0) ;
    glVertex3f(0.0f, 0.0f, -100.0f) ;
    glVertex3f(0.0f, 0.0f, 100.0f) ;
    glColor3f(1.0, 0.0, 0.0) ;
    glVertex3f(-100.0f, 0.0f, 0.0f) ;
    glVertex3f(100.0f, 0.0f, 0.0f) ;
    for (int j=0; j<get_nrows(maze) + 1; ++j) {
        glVertex3f(0.0f, (float) j, 0.0f) ;
        glVertex3f((float) get_ncols(maze), (float) j, 0.0f) ;
    }
    glColor3f(0.0, 1.0, 0.0) ;
    glVertex3f(0.0f, -100.0f, 0.0f) ;
    glVertex3f(0.0f, 100.0f, 0.0f) ;
    // Draw lines parallel to y-axes.
    for (int i=0; i<get_ncols(maze) + 1; ++i) {
        glVertex3f((float) i, 0.0f, 0.0f) ;
        glVertex3f((float) i, (float) get_nrows(maze), 0.0f) ;
    }
    glEnd() ;
    glEnable(GL_LIGHTING) ;

    //TODO remove
    /*
    glBegin(GL_TRIANGLES);
    glVertex3f(1.0f, 0.0f, 5.0f);
    glVertex3f(2.0f, 0.0f, 5.0f);
    glVertex3f(2.0f, 0.0f, 6.0f);
    glEnd();
    */
}


/**
 * Draw a square of side-length 2 in the xy-plane centered at the origin. This
 * is used to color the start and end cells of the maze.
 * @param material - the material of the square.
 */
void draw_square(material_t material) {
    debug("draw_square()");

    // Specify the material for the square.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, material.diffuse) ;
    glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular) ;
    glMaterialf(GL_FRONT, GL_SHININESS, material.phong_exp) ;

    glBegin(GL_QUADS);
    // z=0 plane.
    glNormal3f(0.0, 0.0, 0.0) ;
    glVertex3f(-1.0, -1.0, 0.0) ;
    glVertex3f(1.0, -1.0, 0.0) ;
    glVertex3f(1.0, 1.0, 0.0) ;
    glVertex3f(-1.0, 1.0, 0.0) ;
    glEnd();
}

/**
 * Draw a marker square on the bottom floor of the maze by transforming the
 * draw_square object. The new marker square will be centered at cell given in
 * the parameters.
 * @param row - the row of the maze cell to place the marker.
 * @param col - the column of the maze cell to place the marker.
 * @param material - the material of the square.
 */
void draw_marker(float row, float col, material_t material) {
    // Make sure we're talking about the m-v xfrm stack.
    glMatrixMode(GL_MODELVIEW) ;

    // Push a copy of the current m-v xfrm onto the stack.
    glPushMatrix() ;

    // Define model transform.
    glTranslatef(col + 0.5f, row + 0.5f, 0.0f);
    glScalef(0.25f, 0.25, 1.0f);

    // Draw the square.
    draw_square(material);

    // Undo the model transform.
    glPopMatrix();

}


/**
 * Draw a wall in the maze in the world frame by translating and rotating a
 * draw_square object. A wall is a draw_rect rectangle that sits on the positive x-z plane with length and height 1.25 and width .25.
 * @param row - the row of the maze cell.
 * @param col - the column of the maze cell.
 * @param direction - in which direction to draw the wall. This is an unsigned
 * char defined in maze.h and maze.c.
 */
void draw_wall(float row, float col, unsigned char direction) {
    //debug("draw_wall()");

    // TODO: should this possibly be further up the call sequence?
    // Make sure we're talking about the m-v xfrm stack.
    glMatrixMode(GL_MODELVIEW) ;

    // Push a copy of the current m-v xfrm onto the stack.
    glPushMatrix() ;

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
            assert(0) ;
    }
    // Rotate walls to be parallel perpendicular to x-y plane.
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    // Stretch walls so the overlap cleanly.
    glScalef(1.25f, 1.0f, 1.0f);


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
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gold.diffuse) ;
    glMaterialfv(GL_FRONT, GL_SPECULAR, gold.specular) ;
    glMaterialf(GL_FRONT, GL_SHININESS, gold.phong_exp) ;

    glBegin(GL_QUADS);
    // z=.125 plane.
    glNormal3f(0.0, 0.0, 1.0) ;
    glVertex3f(-0.5, -0.5, 0.125) ;
    glVertex3f(0.5, -0.5, 0.125) ;
    glVertex3f(0.5, 0.5, 0.125) ;
    glVertex3f(-0.5, 0.5, 0.125) ;

    // z=-.125 plane.
    glNormal3f(0.0, 0.0, -1.0) ;
    glVertex3f(-0.5, -0.5, -0.125) ;
    glVertex3f(-0.5, 0.5, -0.125) ;
    glVertex3f(0.5, 0.5, -0.125) ;
    glVertex3f(0.5, -0.5, -0.125) ;

    // x=0.5 plane.
    glNormal3f(1.0, 0.0, 0.0) ;
    glVertex3f(0.5, -0.5, 0.125) ;
    glVertex3f(0.5, -0.5, -0.125) ;
    glVertex3f(0.5, 0.5, -0.125) ;
    glVertex3f(0.5, 0.5, 0.125) ;

    // x=-0.5 plane.
    glNormal3f(-1.0, 0.0, 0.0) ;
    glVertex3f(-0.5, -0.5, 0.125) ;
    glVertex3f(-0.5, 0.5, 0.125) ;
    glVertex3f(-0.5, 0.5, -0.125) ;
    glVertex3f(-0.5, -0.5, -0.125) ;

    // y=0.5 plane.
    glNormal3f(0.0, 1.0, 0.0) ;
    glVertex3f(-0.5, 0.5, 0.125) ;
    glVertex3f(0.5, 0.5, 0.125) ;
    glVertex3f(0.5, 0.5, -0.125) ;
    glVertex3f(-0.5, 0.5, -0.125) ;

    // y=-0.5 plane.
    glNormal3f(0.0, -1.0, 0.0) ;
    glVertex3f(-0.5, -0.5, 0.125) ;
    glVertex3f(-0.5, -0.5, -0.125) ;
    glVertex3f(0.5, -0.5, -0.125) ;
    glVertex3f(0.5, -0.5, 0.125) ;
    glEnd();
}

/**
 * Draw the maze by transforming draw_rect() and draw_square() objects. The maze will be drawn on the x-z plane. 
 */
void draw_maze() {
    debug("draw_maze()");

    cell_t* maze_start = get_start(maze) ;
    cell_t* maze_end = get_end(maze) ;
    
    int maze_width = get_ncols(maze) ;
    int maze_height = get_nrows(maze) ;

    // Draw squares on start and end cell
    draw_marker(maze_start->r, maze_start->c, green_plastic);
    draw_marker(maze_end->r, maze_end->c, red_plastic);

    // Draw the walls.  First draw the east and south exterior walls, then
    // draw any north or west walls of each cell.

    for (float i=0.0f; i<maze_width; ++i) {
        for (float j=0.0f; j<maze_height; ++j) {
            // Draw south wall if height==0
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

/** Draw num_faces of the cube.
 */

void draw_cube() {
    debug("draw_cube()") ;

    // Specify the material for the cube.
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, gold.diffuse) ;
    glMaterialfv(GL_FRONT, GL_SPECULAR, gold.specular) ;
    glMaterialf(GL_FRONT, GL_SHININESS, gold.phong_exp) ;

    // Draw cube as a sequence of GL_QUADS.  Because we are using GL
    // lighting, we must specify the normal to associate to each vertex.
    // The normal is part of the state, so if we want to use the same
    // normal for many vertices, we only have to set the normal once.
    glBegin(GL_QUADS) ;

    // z=1 plane.
    glNormal3f(0.0, 0.0, 1.0) ;
    glVertex3f(-1.0, -1.0, 1.0) ;
    glVertex3f(1.0, -1.0, 1.0) ;
    glVertex3f(1.0, 1.0, 1.0) ;
    glVertex3f(-1.0, 1.0, 1.0) ;

    // z=-1 plane.
    glNormal3f(0.0, 0.0, -1.0) ;
    glVertex3f(-1.0, -1.0, -1.0) ;
    glVertex3f(-1.0, 1.0, -1.0) ;
    glVertex3f(1.0, 1.0, -1.0) ;
    glVertex3f(1.0, -1.0, -1.0) ;

    // x=1 plane.
    glNormal3f(1.0, 0.0, 0.0) ;
    glVertex3f(1.0, -1.0, 1.0) ;
    glVertex3f(1.0, -1.0, -1.0) ;
    glVertex3f(1.0, 1.0, -1.0) ;
    glVertex3f(1.0, 1.0, 1.0) ;

    // x=-1 plane.
    glNormal3f(-1.0, 0.0, 0.0) ;
    glVertex3f(-1.0, -1.0, 1.0) ;
    glVertex3f(-1.0, 1.0, 1.0) ;
    glVertex3f(-1.0, 1.0, -1.0) ;
    glVertex3f(-1.0, -1.0, -1.0) ;

    // y=1 plane.
    glNormal3f(0.0, 1.0, 0.0) ;
    glVertex3f(-1.0, 1.0, 1.0) ;
    glVertex3f(1.0, 1.0, 1.0) ;
    glVertex3f(1.0, 1.0, -1.0) ;
    glVertex3f(-1.0, 1.0, -1.0) ;

    // y=-1 plane.
    glNormal3f(0.0, -1.0, 0.0) ;
    glVertex3f(-1.0, -1.0, 1.0) ;
    glVertex3f(-1.0, -1.0, -1.0) ;
    glVertex3f(1.0, -1.0, -1.0) ;
    glVertex3f(1.0, -1.0, 1.0) ;

    glEnd() ;
}

/** Draw a string on the screen.  The string will be drawn in the current
 *  color and at the current raster position.
 */
void draw_string(char* s) {
    for (char *c=s; *c!='\0'; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_10, *c) ;
    }
}

/** Print the position (distance, theta, and phi) on the screen.
 */
void print_position() {
    glColor3f(0.0f, 0.0f, 0.0f) ;

    glWindowPos2s(10, 30) ;
    char* s ;
    asprintf(&s, "Camera position = (%f, %f, %f)", eye_x, eye_y, eye_z) ;
    draw_string(s) ;
    free(s) ;

    glWindowPos2s(10, 10) ;
    asprintf(&s, "Heading = %f; Heading (rads) = %f", heading, heading*M_PI/180.0f) ;
    draw_string(s) ;
    free(s) ;
}

/** Handle a display request by clearing the screen, drawing the axes
 *  and cube, and printing the viewpoint position.
 */
void handle_display() {
    debug("handle_display()") ;
    if (!bird_eye_state == DOWN) set_bird_eye();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    // Note that the lights are sent through the pipeline, and a light's
    // position is modified by the model-view transformation.  By setting the
    // light's position here, we ensure that the light lives in a fixed
    // place in the world.  See set_lights() for more info.
    glLightfv(GL_LIGHT0, GL_POSITION, far_light.position) ;

    draw_axes();
    
    //draw_rect();
    //draw_square();
    draw_maze();

    print_position() ;

    glFlush() ;
    
}


/** Handle a display request by clearing the screen, drawing the axes
 *  and cube, and printing the viewpoint position.
 */
void handle_animate() {
    debug("handle_display()") ;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

    // Note that the lights are sent through the pipeline, and a light's
    // position is modified by the model-view transformation.  By setting the
    // light's position here, we ensure that the light lives in a fixed
    // place in the world.  See set_lights() for more info.
    glLightfv(GL_LIGHT0, GL_POSITION, far_light.position) ;

    draw_axes();
    
    //draw_rect();
    //draw_square();
    draw_maze();

    print_position() ;

    glFlush() ;
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
            bird_eye_depth = ANIMATE_STEPS;
            bird_eye_toggle();
            break ;
        case '+':
            debug("handle_key() - increase eye dist.");
            eye_z += EYE_DIST_INCR ;
            set_camera() ;
            break ;
        case '-':
            debug("handle_key() - decrease eye dist.");
            eye_z -= EYE_DIST_INCR ;
            set_camera() ;
            break ;
        default:
            break ;
    }
    glutPostRedisplay() ;
}

/** Handle keyboard events:
 *
 *  - LEFT, RIGHT:  increment/decrement heading.
 *  - UP, DOWN:  move eye position.
 *      Move eye_x += EYE_DIST_INCR * sin(heading)
 *      Move eye_y += EYE_DIST_INCR * cos(heading)
 *
 *  Redisplays will be requested for every key event.
 *
 *  @param key the key that was pressed.
 *  @param x the mouse x-position when <code>key</code> was pressed.
 *  @param y the mouse y-position when <code>key</code> was pressed.
 */
void handle_special_key(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            heading -= HEADING_INCR;
            if (heading < 0.0f) heading += 360.0f ;
            debug("GLUT_KEY_LEFT: heading = %f", heading);
            break ;
        case GLUT_KEY_RIGHT:
            debug("GLUT_KEY_RIGHT: heading = %f", heading);
            heading += HEADING_INCR;
            if (heading >= 360.0f) heading -= 360.0f ;
            break ;
        case GLUT_KEY_UP:
            eye_x += EYE_DIST_INCR*((float) sin((double) (heading*M_PI/180.0f) ));
            eye_y += EYE_DIST_INCR*((float) cos((double) (heading*M_PI/180.0f) )) ;
            debug("GLUT_KEY_UP: eye_x = %f, eye_y = %f", eye_x, eye_y);
            break ;
        case GLUT_KEY_DOWN:
            eye_x -= EYE_DIST_INCR*((float) sin((double) (heading*M_PI/180.0f) ));
            eye_y -= EYE_DIST_INCR*((float) cos((double) (heading*M_PI/180.0f) )) ;
            debug("GLUT_KEY_DOWN: eye_x = %f, eye_y = %f", eye_x, eye_y);
            break ;
        default:
            break ;
    }
    set_camera() ;
    glutPostRedisplay() ;

}
