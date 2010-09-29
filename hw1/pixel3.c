/** @file pixel3.c
 *  @brief Sample application that draws two rectangles in a window
 *      by directly setting pixel values in the framebuffer, and
 *      then moves them back and forth.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Defaults for the window.
#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600
int win_width, win_height ;

// GLUT callbacks.
void draw_upper_rectangle(void) ;                 // Display callback.
void draw_lower_rectangle(void) ;                 // Display callback.
void no_op() ;
void handle_reshape(int w, int h) ;         // Reshape callback.
void update_left_pos(void) ;                // Idle callback.

// GLUT window IDs
int upper_win, lower_win ;

// Drawing parameters.
#define MIN_LEFT .1f
#define MAX_LEFT .6f
#define MOVE_INCR .01
// Which direction the rectangle is moving.
enum {MOVE_LEFT, MOVE_RIGHT} move_dir ;
// How far from the left to draw the rectangle, as a percentage of the 
// window width.
float left_pos = MIN_LEFT ;        
// What color to draw.  Here we're doing something bad:  taking advantage
// of the fact that enumerated type elements are ints starting at 0.
typedef enum {RED, GREEN, BLUE} color_t ;


// A marginally-clever debugging function that expends to a no-op
// when NDEBUG is defined and otherwise prints its string formatting
// arguments to stderr.
#ifdef NDEBUG
#define debug(fmt, ...) 
#else
void debug(const char* fmt, ...) {
    va_list argptr ;
    va_start(argptr, fmt) ;

    char* fmt_newline ;
    asprintf(&fmt_newline, "%s\n", fmt) ;
    vfprintf(stderr, fmt_newline, argptr) ;
    free(fmt_newline) ;
}
#endif

int main(int argc, char **argv) {

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
    glutInitWindowPosition(0, 0) ;
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;

    // Create the main window, set display callback to a no-op.
    int main_win = glutCreateWindow("Simple pixel-oriented graphics") ;
    glutDisplayFunc(no_op) ;
    int win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    int win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create upper and lower subwindows, set appropriate display callbacks.
    upper_win = glutCreateSubWindow(main_win, 0, 0, win_width, win_height/2) ;
    glutDisplayFunc(draw_upper_rectangle) ;
    lower_win = glutCreateSubWindow(main_win, 0, win_height/2, win_width,
            win_height-win_height/2) ;
    glutDisplayFunc(draw_lower_rectangle) ;

    glutIdleFunc(update_left_pos) ;

    // Enter the main event loop.
    glutMainLoop() ;

    return EXIT_SUCCESS ;
}

/** Handle display events.  We draw a rectangle of width/height
 *  approximately 1/3 the width/height of the window.  The rectangle is
 *  vertically centered and its left edge is about 1/10 of the widow
 *  width from the left side of the window.
 *
 *  @param color the color to draw the rectangle.
 */
void draw_rectangle(color_t color) {
    debug("draw_rectangle(%d)", color) ;

    win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    win_height = glutGet(GLUT_WINDOW_HEIGHT) ;

    // Create the framebuffer, initialize to all zero (i.e., black).
    GLubyte fb[win_height][win_width][3] ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLubyte)) ;

    // Fill the framebuffer with the rectangle pixels.
    for (int h=win_height/3; h<2*win_height/3; ++h) {
        for (int w=0; w<win_width/3; ++w) {
            fb[h][(int)(win_width*left_pos)+w][color] = 255 ;
        }
    }

    // Draw the pixels.  An alternative to WindowPos* is RasterPos*, but the
    // latter takes into account the current model-view transformation, so
    // doesn't seem appropriate.  We should probably always specify 
    // GL_UNPACK_ALIGNMENT to be certain that we read the "next" row 
    // of pixels correctly.
    glWindowPos2s(0, 0) ;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_UNSIGNED_BYTE, fb) ;

    glFlush() ;

    glutSwapBuffers() ;

}

/** Handle display events for the upper window.  Draw a red rectangle
 *  using draw_rectangle.
 */
void draw_upper_rectangle() {
    draw_rectangle(RED) ;
}

/** Handle display events for the upper window.  Draw a green rectangle
 *  using draw_rectangle.
 */
void draw_lower_rectangle() {
    draw_rectangle(GREEN) ;
}

/** Handle reshape events.  All we do here is record the new width and
 *  height.
 *
 *  @param w the width of the resized window.
 *  @param h the height of the resized window.
 */
void handle_reshape(int w, int h) {
    debug("handle_reshape(%d, %d)", w, h) ;
    win_width = w ; 
    win_height = h ;
}

/** Update the position of the rectangle.  The position starts at MOVE_LEFT,
 *  then increases to MOVE_RIGHT in increments of MOVE_INCR, then goes back, 
 *  and keeps going forever.
 */
void update_left_pos() {
    debug("update_left_pos()") ;
    switch (move_dir) {
        case MOVE_RIGHT:
            if (left_pos >= MAX_LEFT) move_dir = MOVE_LEFT ;
            else left_pos += MOVE_INCR ;
            break ;
        case MOVE_LEFT:
            if (left_pos <= MIN_LEFT) move_dir = MOVE_RIGHT ;
            else left_pos -= MOVE_INCR ;
            break ;
        default:
            assert(0) ;
    }

    glutSetWindow(upper_win) ;
    glutPostRedisplay() ;
    glutSetWindow(lower_win) ;
    glutPostRedisplay() ;

}

void no_op() {
    // glClear(GL_COLOR_BUFFER_BIT) ;
}