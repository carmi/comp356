/**
 *  @file hw3.c
 *  @brief - Antialiased midpoint algorithm demonstration.
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #3
 *  Evan Carmi (WesID: 807136)
 *  ecarmi@wesleyan.edu
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "debug.h"

// Window data.
const int DEFAULT_WIN_WIDTH = 800;
const int DEFAULT_WIN_HEIGHT = 600;
int win_width;
int win_height;

// Non-display callbacks.
void handle_resize(int, int);
void handle_menu(int);
void handle_line_select(int, int, int, int);

// Display callbacks.
void display_unzoom();
void display_zoom();

// Application function declarations.
void draw_unzoom_fb();
void draw_unzoom_background();
void draw_zoom_fb();
void handle_exit();

// Utility function declarations.
void draw_pixel(int x, int y, int area);

// Menu entries.
int create_menu();
enum menu_t {MENU_CLEAR, MENU_ZOOM, MENU_ALIAS, MENU_EXIT};
void reset();

// The in-memory copy of the framebuffers; one for the unzoomed view and
// one for the zoomed view.  We waste space and simplify code a bit by
// drawing both whenever a line is specified, and then just switching
// which one to display when the Antialias/alias menu is selected.
GLfloat *unzoom_fb, *zoom_fb;

// Application state.

// Zooming parameters.
#define ZOOM_RECT_WIDTH 25
#define ZOOM_RECT_HEIGHT 25
#define ZOOM_FACTOR 20
int zoom_llx, zoom_lly;
int zoom_urx, zoom_ury;
bool do_zoom;
bool enable_zoom;

// Line segment endpoints.
int line_coord_idx = 0;
int line_coord[2][2];
bool do_line = false;

// Anti-aliasing parameters.
bool do_antialias = false;

int main(int argc, char **argv) {

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Create the main window (reset() sets the display callback).
    glutCreateWindow("Antialiased Bressenham Line Drawing");
    glutReshapeFunc(handle_resize);
    create_menu();
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    // GL initialization
    glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(GL_FLOAT));

    // Application initialization
    win_width = glutGet(GLUT_WINDOW_WIDTH);
    win_height = glutGet(GLUT_WINDOW_HEIGHT);
    reset();

    // Enter the main event loop.
    atexit(handle_exit);
    glutMainLoop();

    return EXIT_SUCCESS;
}

/** At exit, free any dynamically allocated memory.
 */
void handle_exit() {
    debug("handle_exit()");
    if (unzoom_fb != NULL) free(unzoom_fb);
    if (zoom_fb != NULL) free(zoom_fb);
}

/** Create the main menu.
 */
int create_menu() {
    int menu_id = glutCreateMenu(handle_menu);

    glutAddMenuEntry("Clear window", MENU_CLEAR);
    glutAddMenuEntry("Zoom in/out", MENU_ZOOM);
    glutAddMenuEntry("Antialias/alias", MENU_ALIAS);
    glutAddMenuEntry("Exit", MENU_EXIT);

    return menu_id;
}

/** Handle menu selection.
 *
 *  @param item the menu item that was selected.
 */
void handle_menu(int item) {
    switch ((enum menu_t)item) {
        case MENU_CLEAR:
            reset();
            glutPostRedisplay();
            break;
        case MENU_ZOOM:
            if (enable_zoom) {
                do_zoom = !do_zoom;
                if (do_zoom) glutDisplayFunc(display_zoom);
                else glutDisplayFunc(display_unzoom);
                glutPostRedisplay();
            }
            break;
        case MENU_ALIAS:
            do_antialias = !do_antialias;
            draw_unzoom_fb();
            draw_zoom_fb();
            glutPostRedisplay();
            break;
        case MENU_EXIT:
            exit(EXIT_SUCCESS);
        default:
            assert(0);
    }
}

/** Handle a resize event by recording the new width and height,
 *  reallocating the framebuffer to the new window size, and initialization
 *  it to all white.
 *  
 *  @param width the new width of the window.
 *  @param height the new height of the window.
 */
void handle_resize(int width, int height) {

    debug("handle_resize(%d, %d)\n", width, height);
    win_width = width;
    win_height = height;

}

/** Handle left-button-up events as defining the first and second
 *  points of the line segment to draw.
 *
 *  @param button the button for the event.  We only care about
 *      <code>GLUT_LEFT_BUTTON</code>.
 *  @param state the state of the button.  We only care about
 *      <code>GLUT_UP</code>.
 *  @param x the x-coordinate of the button event in pixels (GLUT
 *      coordinates, so (0, 0) is the upper-left corner of the window).
 *  @param y the y-coordinate of the button event in pixels (GLUT
 *      coordinates, so (0, 0) is the upper-left corner of the window).
 */
void handle_line_select(int button, int state, int x, int y) {
    debug("handle_line_select()");
    if (button != GLUT_LEFT_BUTTON || state != GLUT_UP) return;
    
    line_coord[line_coord_idx][0] = x;
    line_coord[line_coord_idx][1] = win_height - y;
    ++line_coord_idx;
    if (line_coord_idx == 2) {
        enable_zoom = true;
        draw_unzoom_fb();
        draw_zoom_fb();
        glutMouseFunc(NULL);
        glutPostRedisplay();
    }

}

/** Reset the application by setting the display callback to just display
 *  the "background" framebuffer and zoom out.
 */
void reset() {
    do_zoom = false;
    enable_zoom = false;
    line_coord_idx = 0;
    draw_unzoom_background();
    glutDisplayFunc(display_unzoom);
    glutMouseFunc(handle_line_select);
}

/** Get the offset into the frame-buffer for a given pixel position.
 *  
 *  @param y the y-coordinate (row) of the pixel.
 *  @param x the x-coordinate (column) of the pixel.
 *  @param c the color component.
 *
 *  @return the value i such that fb+i is the framebuffer element to
 *      set for color c of pixel (x, y).
 */
int fb_offset(int y, int x, int c) {
    return y*(win_width*3) + x*3 + c;
}

//
// LINE-DRAWING FUNCTIONS
//

/** Draw the line segment using the standard incremental Bressenham algorithm.
 */
void draw_line_aliased() {
    debug("draw_line_aliased()");

    // Bressenham's algorithm for line-drawing.

    int x0 = line_coord[0][0], y0 = line_coord[0][1];
    int x1 = line_coord[1][0], y1 = line_coord[1][1];

    float xIncr = x1 - x0;
    float yIncr = y0 - y1;
    float bigIncr = xIncr + yIncr;
    float C = x0*y1 - x1*y0;

    int y = y0;
    float d = yIncr*(x0+1.0f) + xIncr*(y0+.5f) + C;
    debug("d: %f", d);
    for (int x=x0; x<=x1; ++x) {
        debug("x: %i", x);
        *(unzoom_fb+fb_offset(y, x, 0)) = 0.0f;
        *(unzoom_fb+fb_offset(y, x, 1)) = 0.0f;
        *(unzoom_fb+fb_offset(y, x, 2)) = 0.0f;

        if (d < 0) {
            debug("d1: %f", d);
            d += bigIncr;
            y += 1;
        }
        else {
            debug("d2: %f", d);
            d += yIncr;
        }
    }

}

/** Draw the line segment using the super-sampled midpoint algorithm for
 *  antialiasing.
 */
void draw_line_antialiased() {
    debug("draw_line_antialiased()");
    // Bressenham's algorithm for line-drawing.

    // Draw a line from x0, y0 - .5 to x1, y1 - .5
    int x0 = line_coord[0][0];
    float y0 = ((float) line_coord[0][1]) - 0.5f;
    int x1 = line_coord[1][0];
    float y1 = ((float) line_coord[1][1]) - 0.5f;

    float xIncr = (float) x1 - (float) x0;
    float yIncr = y0 - y1;
    float bigIncr = xIncr + yIncr;
    float C = x0*y1 - x1*y0;

    int y = (int) y0;
    // Calculate d at the midpoint of the first subpixel we'll look at, this
    // midpoint's x-value is at x0, because we want to start at the subpixel
    // (0, 2). And the midpoint's y-value is y0+.1, because we want to start
    // at the middle of this subpixel (0, 2).
    float d = yIncr*(x0) + xIncr*(y0+0.1f) + C;
    debug("d: %f", d);

    // Create a counter, y_counter, to keep track of our position in subpixels
    // y-grid.
    int y_counter = 0;

    for (int x=x0; x<=x1; ++x) {
        // For each x-value, we go into it's 5x5 subpixel grid and calculate
        // areas of subpixels above and below our line.
        int subpixels_below = 0;
        int subpixels_above = 0;

        // Iterate through subpixel grid.
        for (int sub_x = 0; sub_x < 5; ++sub_x) {

            // Edge Case: Lines "start" from the middle of their pixels, so if
            // this is first pixel, only calculate area of half the pixel - 3
            // columns.
            if (x == x0 && sub_x == 0) {
                // If first pixel, start at sub_x = 2.
                sub_x = 2;
            }

            if (d < 0) {
                // If d<0, the line lies above the midpoint of our subpixel, so
                // increment subpixel space's y; i.e. our y_counter.
                d += bigIncr/5.0f;

                if (y_counter < 4) {
                    y_counter += 1;

                // If y_counter == 4, increment framebuffer y, otherwise
                // increment y_counter
                } else if (y_counter == 4) {
                    // We cross a horizontal line, so draw the bottom pixel and
                    // then move onto the next two pixels above.

                    // Draw bottom pixel.
                    draw_pixel(x, y+1, subpixels_below);
                    // Now move up and work on next two pixels. Swap subpixel
                    // area count and set subpixels_above to zero and reset
                    // y_counter.
                    subpixels_below = subpixels_above;
                    subpixels_above = 0;
                    y_counter = 0;
                    y += 1;
                } else {
                    debug("We shouldn't be here, quit.");
                    assert(0);
                }
            }
            else {
                // Line lies below midpoint, increment d by yIncr/5
                d += yIncr/5.0f;
            }

            // After each iteration through a subpixel column, we add 5
            // subpixels to the areas. We know the number of subpixels in this
            // columnn that are below the line, this is y_counter, and 
            // conveniently, we therefore know that the number of subpixels
            // above the line are therefore 5 - the subpixels below.
            subpixels_above += (5 - y_counter);
            subpixels_below += (y_counter);

            // Edge Case: If this is the last pixel (x == x1), only calculate
            // area for 3 subpixel columns, and then stop.
            if (x == x1 && sub_x == 2) {
                sub_x = 4;
            }
        }
        // When we cross a vertical line between two pixels (whenever we
        // increment x), we draw two pixels. Because our line starts at 
        // y0 - .5, our pixels' y-values are y+1 and y+2
        draw_pixel(x, y+1, subpixels_above);
        draw_pixel(x, y+2, subpixels_below);
    }
}

/**
 * Draw a pixel with a specific shading.
 * @param x - the x-coordinate of the pixel to be drawn.
 * @param y - the y-coordinate of the pixel to be drawn.
 * @param area - the number of subpixels (1 pixel can be broken down into a 5x5
 *      subpixel grid) that are covered by the line. We then shade the pixel a
 *      color according to 1 - area/25. Precondition: 0 <= area <= 25.
 */
void draw_pixel(int x, int y, int area) {
    debug("draw_pixel() x: %i, y: %i, area: %i", x, y, area);
    float shading = 1 - (float) area / 25.0f;
    *(unzoom_fb+fb_offset(y, x, 0)) = shading;
    *(unzoom_fb+fb_offset(y, x, 1)) = shading;
    *(unzoom_fb+fb_offset(y, x, 2)) = shading;
}

/** Draw the line.
 */
void draw_line() {
    if (do_antialias) draw_line_antialiased();
    else draw_line_aliased();
}

//
// DISPLAY CALLBACK FUNCTIONS
//

/** Draw the background into the unzoomed framebuffer.
 */
void draw_unzoom_background() {

    // Unconditionally reallocate the framebuffer.
    if (unzoom_fb != NULL) free(unzoom_fb);
    debug("draw_unzoom_background():  allocating in-memory framebuffer");
    unzoom_fb = malloc(win_width*win_height*3*sizeof(GLfloat));

    // Corners of the zoom rectangle.
    zoom_llx = (win_width-ZOOM_RECT_WIDTH)/2;
    zoom_lly = (win_height-ZOOM_RECT_HEIGHT)/2;
    zoom_urx = zoom_llx + ZOOM_RECT_WIDTH -1;
    zoom_ury = zoom_lly + ZOOM_RECT_HEIGHT - 1;

    // Set the background to white.
    for (int i=0; i<win_width*win_height*3; ++i) *(unzoom_fb+i) = 1.0f;

    // Draw a blue zoom rectangle.
    for (int i=zoom_llx; i<=zoom_urx; ++i) {
        *(unzoom_fb+fb_offset(zoom_lly, i, 0)) = 0.0f;
        *(unzoom_fb+fb_offset(zoom_lly, i, 1)) = 0.0f;
        *(unzoom_fb+fb_offset(zoom_ury, i, 0)) = 0.0f;
        *(unzoom_fb+fb_offset(zoom_ury, i, 1)) = 0.0f;
    }
    for (int i=zoom_lly; i<=zoom_ury; ++i) {
        *(unzoom_fb+fb_offset(i, zoom_llx, 0)) = 0.0f;
        *(unzoom_fb+fb_offset(i, zoom_llx, 1)) = 0.0f;
        *(unzoom_fb+fb_offset(i, zoom_urx, 0)) = 0.0f;
        *(unzoom_fb+fb_offset(i, zoom_urx, 1)) = 0.0f;
    }
}

/** Draw the unzoomed framebuffer.
 */
void draw_unzoom_fb() {
    draw_unzoom_background();
    draw_line();
}

/** Draw the zoomed framebuffer.
 */
void draw_zoom_fb() {
    if (zoom_fb != NULL) free(zoom_fb);
    zoom_fb = malloc(win_width*win_height*3*sizeof(GL_FLOAT));

    // Set the background to white.
    for (int i=0; i<win_width*win_height*3; ++i) *(zoom_fb+i) = 1.0f;

    int llx = win_width/2 - ZOOM_RECT_WIDTH*ZOOM_FACTOR/2;
    int lly = win_height/2 - ZOOM_RECT_HEIGHT*ZOOM_FACTOR/2;
    
    for (int x=0; x<ZOOM_RECT_WIDTH; ++x) {
        for (int y=0; y<ZOOM_RECT_HEIGHT; ++y) {
            for (int i=0; i<ZOOM_FACTOR; ++i) {
                for (int j=0; j<ZOOM_FACTOR; ++j) {
                    for (int c=0; c<3; ++c)
                        *(zoom_fb+fb_offset(lly+ZOOM_FACTOR*y+j,
                                    llx+ZOOM_FACTOR*x+i, c)) = 
                            *(unzoom_fb+fb_offset(zoom_lly+y, zoom_llx+x, c));
                }
            }
        }
    }

}

/** Draw the specified framebuffer.
 *  
 *  @param fb the framebuffer to draw.  Must have size
 *      <code>win_width*win_height*3</code>.
 */
void draw_pixels(GLfloat* fb) {
    glWindowPos2s(0, 0);
    glDrawPixels(win_width, win_height, GL_RGB, GL_FLOAT, fb);
    glFlush();
    glutSwapBuffers();
}

/** Display the unzommed framebuffer.
 */
void display_unzoom() {
    draw_pixels(unzoom_fb);
}

/** Display the zoomed framebuffer.
 */
void display_zoom() {
    draw_pixels(zoom_fb);
}
