/** @file hw2bp1.c
 *  @brief - A ray tracing algorithm written in C.
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #2B
 *  Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
 *  ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef NDEBUG
    #include <stdarg.h>
#endif
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

#include "stack356.h"
#include "list356.h"
#include "geom356.h"
#include "surfaces_lights.h"

#include "debug.h"

#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600

// Window dimensions.
int win_width, win_height;

// GLUT Function Declarations
void no_display(void);
void handle_reshape(int w, int h);

// Function Declarations
void set_camera_frame(point3_t* e, point3_t* P, vector3_t* up);
void get_dir_vec(float i, float j, vector3_t* result);



// Window identifiers.
int main_win;    // Main top-level window.

// Framebuffer identifier
float* fb;

// Surfaces and Lights identifiers
list356_t* surfaces;
list356_t* lights;

// View Data identifiers
point3_t* eye;
point3_t* look_at_point;
vector3_t* up_dir;

// View Plane values
float view_plane_dist;
float view_plane_width;
float view_plane_height;

// Camera Frame Basis
vector3_t* u;
vector3_t* v;
vector3_t* w;

int main(int argc, char **argv) {
    // Get surfaces and lights from surfaces_lights.c
    surfaces = get_surfaces();
    lights = get_lights();

    // Set view_data and view_plane
    set_view_data(eye, look_at_point, up_dir);
    set_view_plane(&view_plane_dist, &view_plane_width, &view_plane_height);

    // Calculate camera frame
    set_camera_frame(eye, look_at_point, up_dir);

    // Initialize the drawing window.
    debug("main(): initialize main window.") ;
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Create the main window, set the display callback.
    debug("main(): create main window.") ;
    main_win = glutCreateWindow("A Ray Tracer");
    glutDisplayFunc(no_display);
    //glutDisplayFunc(draw_image);
    glutReshapeFunc(handle_reshape);
    
    // Enter the main event loop.
    debug("main(): enter main loop.") ;
    glutMainLoop();
    
    return EXIT_SUCCESS;
}

/**
 * Display callback that iterates through framebuffer and ray traces.
 */
void draw_image() {
    /*
    foreach pixel in framebuffer do:
        ray3_t ray;
        ray->base = eye;
        ray->dir = get_dir_vec(pixel_i, pixel_j)
        color = ray_trace(ray, t0, t1, srec, ...)
        set pixel in framebuffer to color
    */
}

/** Display callback that just clears the window to the clear color.
 */
void no_display() {
    glClear(GL_COLOR_BUFFER_BIT) ;
}

/** Handle reshape events.  This is also run on first initialization of window.
 *  Here we create our in-memory framebuffer.
 *
 *  @param w the width of the resized window.
 *  @param h the height of the resized window.
 */
void handle_reshape(int w, int h) {
    debug("handle_reshape(%d, %d)", w, h) ;
    win_width = w ; 
    win_height = h ;

    // Create framebuffer - 3 dimensional array of float
    fb = malloc(win_width * win_height * 3 * sizeof(float));
    bzero(fb, (win_width*win_height*3)*sizeof(float));

}

/**
 * Calculate and set camera frame.
 *
 * @param eye - the viewpoint
 * @param look_at_point - the look at point
 * @param up-direction - the up direction
 *
 * @result the camera frame bases, u, v, w are defined.
 *
 * Use notation from text and hw2a.
 */
void set_camera_frame(point3_t* e, point3_t* P, vector3_t* up) {
    // Calculate w
    pv_subtract(e, P, w);
    normalize(w);

    // Calculate u
    cross(up, w, u);
    normalize(u);

    // Calculate v
    cross(w, u, v);
}


/**
 * Map a screen pixel (i, j) to a point (in the world frame) on our view
 * rectangle/plane and calculate the vector from this point to our viewpoint.
 *
 * @param i - the column of the pixel.
 * @param j - the row of the pixel.
 * @param result - pointer to vector that will be assigned the result in world
 *                 frame.
 */
void get_dir_vec(float i, float j, vector3_t* result) {
    float x, y, z;
    x = (-view_plane_width/2) + ( ((i + 0.5)/win_width)*view_plane_width);
    y = (-view_plane_height/2) + ( ((j + 0.5)/win_height)*view_plane_height);
    z = -view_plane_dist;

    // result is vector d in Shirley and Marschner notation.
    // Perform a change of basis on the vector result from camera frame to
    // world frame.
    vector3_t tmp_x, tmp_y, tmp_z;
    multiply(u, x, &tmp_x);
    multiply(v, y, &tmp_y);
    multiply(w, z, &tmp_z);
    // Now sum tmp_x, tmp_y, tmp_z
    add(&tmp_x, &tmp_y, &tmp_x);
    add(&tmp_x, &tmp_z, result);
}
