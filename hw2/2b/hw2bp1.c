/** @file hw2p1.c
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

#include "debug.h"

#define MAX_STRING_LENGTH 256
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600

// Window dimensions.
int win_width, win_height;

// GLUT Function Declarations
void no_display(void);

// Window identifiers.
int main_win;    // Main top-level window.


int main(int argc, char **argv) {
    debug("main(): initialize main window.") ;
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    debug("main(): create main window.") ;
    main_win = glutCreateWindow("A Ray Tracer");
    glutDisplayFunc(no_display);
    
    // Enter the main event loop.
    debug("main(): enter main loop.") ;
    glutMainLoop();
    
    return EXIT_SUCCESS;
}

/** Display callback that just clears the window to the clear color.
 */
void no_display() {
    glClear(GL_COLOR_BUFFER_BIT) ;
}
