/** Antialiased midpoint algorithm demonstration.
 *
 *  @author N. Danner
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
const int DEFAULT_WIN_WIDTH = 800 ;
const int DEFAULT_WIN_HEIGHT = 600 ;
int win_width ;
int win_height ;

// Non-display callbacks.
void handle_resize(int, int) ;
void handle_menu(int) ;
void handle_line_select(int, int, int, int) ;

// Display callbacks.
void display_unzoom() ;
void display_zoom() ;

// Application function declarations.
void draw_unzoom_fb() ;
void draw_unzoom_background() ;
void draw_zoom_fb() ;
void handle_exit() ;

void draw_pixel_antialiased(int fb_x, float y_start, float y_end, int dir);
int int_part(float value) ;
float frac_part(float value) ;
// Menu entries.
int create_menu() ;
enum menu_t {MENU_CLEAR, MENU_ZOOM, MENU_ALIAS, MENU_EXIT} ;
void reset() ;

// The in-memory copy of the framebuffers; one for the unzoomed view and
// one for the zoomed view.  We waste space and simplify code a bit by
// drawing both whenever a line is specified, and then just switching
// which one to display when the Antialias/alias menu is selected.
GLfloat *unzoom_fb, *zoom_fb ;

// Application state.

// Zooming parameters.
#define ZOOM_RECT_WIDTH 25
#define ZOOM_RECT_HEIGHT 25
#define ZOOM_FACTOR 20
int zoom_llx, zoom_lly ;
int zoom_urx, zoom_ury ;
bool do_zoom ;
bool enable_zoom ;

// Line segment endpoints.
int line_coord_idx = 0 ;
int line_coord[2][2] ;
bool do_line = false ;

// Anti-aliasing parameters.
bool do_antialias = false ;

// Antialiased line drawing enumerations
enum dir { UP, DOWN};

int main(int argc, char **argv) {

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
    glutInitWindowPosition(0, 0) ;
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;

    // Create the main window (reset() sets the display callback).
    glutCreateWindow("Antialiased Bressenham line drawing") ;
    glutReshapeFunc(handle_resize) ;
    create_menu() ;
    glutAttachMenu(GLUT_RIGHT_BUTTON) ;

    // GL initialization
    glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(GL_FLOAT)) ;

    // Application initialization
    win_width = glutGet(GLUT_WINDOW_WIDTH) ;
    win_height = glutGet(GLUT_WINDOW_HEIGHT) ;
    reset() ;

    // Enter the main event loop.
    atexit(handle_exit) ;
    glutMainLoop() ;

    return EXIT_SUCCESS ;
}

/** At exit, free any dynamically allocated memory.
 */
void handle_exit() {
    debug("handle_exit()") ;
    if (unzoom_fb != NULL) free(unzoom_fb) ;
    if (zoom_fb != NULL) free(zoom_fb) ;
}

/** Create the main menu.
 */
int create_menu() {
    int menu_id = glutCreateMenu(handle_menu) ;

    glutAddMenuEntry("Clear window", MENU_CLEAR) ;
    glutAddMenuEntry("Zoom in/out", MENU_ZOOM) ;
    glutAddMenuEntry("Antialias/alias", MENU_ALIAS) ;
    glutAddMenuEntry("Exit", MENU_EXIT) ;

    return menu_id ;
}

/** Handle menu selection.
 *
 *  @param item the menu item that was selected.
 */
void handle_menu(int item) {
    switch ((enum menu_t)item) {
        case MENU_CLEAR:
            reset() ;
            glutPostRedisplay() ;
            break ;
        case MENU_ZOOM:
            if (enable_zoom) {
                do_zoom = !do_zoom ;
                if (do_zoom) glutDisplayFunc(display_zoom) ;
                else glutDisplayFunc(display_unzoom) ;
                glutPostRedisplay() ;
            }
            break ;
        case MENU_ALIAS:
            do_antialias = !do_antialias ;
            draw_unzoom_fb() ;
            draw_zoom_fb() ;
            glutPostRedisplay() ;
            break ;
        case MENU_EXIT:
            exit(EXIT_SUCCESS) ;
        default:
            assert(0) ;
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

    debug("handle_resize(%d, %d)\n", width, height) ;
    win_width = width ;
    win_height = height ;

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
    debug("handle_line_select()") ;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_UP) return ;
    
    line_coord[line_coord_idx][0] = x ;
    line_coord[line_coord_idx][1] = win_height - y ;
    ++line_coord_idx ;
    if (line_coord_idx == 2) {
        enable_zoom = true ;
        draw_unzoom_fb() ;
        draw_zoom_fb() ;
        glutMouseFunc(NULL) ;
        glutPostRedisplay() ;
    }

}

/** Reset the application by setting the display callback to just display
 *  the "background" framebuffer and zoom out.
 */
void reset() {
    do_zoom = false ;
    enable_zoom = false ;
    line_coord_idx = 0 ;
    draw_unzoom_background() ;
    glutDisplayFunc(display_unzoom) ;
    glutMouseFunc(handle_line_select) ;
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
    return y*(win_width*3) + x*3 + c ;
}

//
// LINE-DRAWING FUNCTIONS
//

/** Draw the line segment using the standard incremental Bressenham algorithm.
 */
void draw_line_aliased() {
    debug("draw_line_aliased()") ;
    // Bressenham's algorithm for line-drawing.

    int x0 = line_coord[0][0], y0 = line_coord[0][1] ;
    int x1 = line_coord[1][0], y1 = line_coord[1][1] ;

    float xIncr = x1 - x0 ;
    float yIncr = y0 - y1 ;
    float bigIncr = xIncr + yIncr ;
    float C = x0*y1 - x1*y0 ;

    int y = y0 ;
    float d = yIncr*(x0+1.0f) + xIncr*(y0+.5f) + C ;
    debug("d: %f", d);
    for (int x=x0; x<=x1; ++x) {
        debug("x: %i", x);
        *(unzoom_fb+fb_offset(y, x, 0)) = 0.0f ;
        *(unzoom_fb+fb_offset(y, x, 1)) = 0.0f ;
        *(unzoom_fb+fb_offset(y, x, 2)) = 0.0f ;

        if (d < 0) {
            debug("d1: %f", d);
            d += bigIncr ;
            y += 1 ;
        }
        else {
            debug("d2: %f", d);
            d += yIncr ;
        }
    }

}

/** Draw the line segment using the super-sampled midpoint algorithm for
 *  antialiasing.
 */
void draw_line_antialiased() {

    int x0 = line_coord[0][0], y0 = line_coord[0][1] ;
    int x1 = line_coord[1][0], y1 = line_coord[1][1] ;

    float xIncr = x1 - x0 ;
    float yIncr = y0 - y1 ;
    float bigIncr = xIncr + yIncr ;
    float C = x0*y1 - x1*y0 ;

    float y = (float) y0 ;
    float d_top = yIncr*(x0+1.0f) + xIncr*(y0 + 0.5f) + C ;
    float d_bot = yIncr*(x0+0.0f) + xIncr*(y0 - 0.5f) + C ;

    // We'll draw two lines each .5 below and above our actual line.
    float slope = (float) (y1 - y0) / (float) (x1 - x0);
    // y = mx +b; b = y - mx; b = intercept.
    float intercept = ((float) y0) - slope*((float)x0);
    // Our first line is .5 above our ideal line, so intercept +.5
    float intercept_top = intercept + .5f;
    //And -.5 for the line below.
    float intercept_bot = intercept - .5f;

    for (int x=x0; x<=x1; ++x) {
        // For each x coordinate of our line, using y=mx+b slope formula, find
        // the y value of the line's intersection with the left pixel border
        // (the y-value when we plug in the x from our for loop.
        float y_top = slope*((float)x) + intercept_top;
        float y_bot = slope*((float)x) + intercept_bot;
        debug("y_top: %f", y_top);
        debug("y_bot: %f", y_bot);

        float y_top_end = slope*(float)(x + 1) + intercept_top;
        float y_bot_end = slope*(float)(x + 1) + intercept_bot;

        draw_pixel_antialiased(x, y_top, y_top_end, 0);
        draw_pixel_antialiased(x, y_bot, y_bot_end, 1);

        /*
        // Calculate area of supersampled pixel.
        float area_top = 0.0f;
        float area_bot = 0.0f;
        for (int x_ss = 0; x_ss < 5; ++x_ss) {

            
            if (d_bot < 0) {
                // the line lies above the midpoint of the next increment.
                y_col += 1;
                area_bot += (5.0f*(y_bot - (int) y_bot))*0.04f; // add 1/25.
                d_bot += bigIncr/5.0f ;
                y_bot += 0.2f ;
            }
            else {
                d_bot += yIncr/5.0f ;
            }
        }
        debug("area_top: %f", area_top);
        debug("area_bot: %f", area_bot);
        debug("y_top: %f", y_top);

        *(unzoom_fb+fb_offset(y_top, x, 0)) = 1.0f - area_top;
        *(unzoom_fb+fb_offset(y_top, x, 1)) = 1.0f - area_top;
        *(unzoom_fb+fb_offset(y_top, x, 2)) = 1.0f - area_top;

        *(unzoom_fb+fb_offset(y_bot, x, 0)) = 1.0f - area_bot;
        *(unzoom_fb+fb_offset(y_bot, x, 1)) = 1.0f - area_bot;
        *(unzoom_fb+fb_offset(y_bot, x, 2)) = 1.0f - area_bot; 
        */

    }

}

/**
 * Draw a pixel with a given degree of grayness depending on the area that is 
 * @param y_start - y-value beginning coordinate in the subpixel grid system.
 * @param y_end - y-value ending coordinate in the subpixel grid system.
 *    // We break the pixel into a 5x5 grid and consider it as a separate
 *    "frame". In this frame we find the area using Bressenham's alg.
 * y_start is start of line in 5x5 grid frame.
 */
void draw_pixel_antialiased(int fb_x, float y_start, float y_end, int dir) {
    debug("draw_pixel_antialiased()") ;
    // Adapted draw_line_aliased() function

    int fb_y = int_part(y_start);
    debug("fb_y: %i", fb_y);
    debug("frac(y_start): %f", frac_part(y_start));
    int y = (int) (frac_part(y_start)*5.0f);
    debug("y: %i", y);
    // Bressenham's algorithm for line-drawing.

    int x0 = 1, x1 = 5;
    float y0 = y_start, y1 = y_end;

    float xIncr = x1 - x0 ;
    float yIncr = y0 - y1 ;
    float bigIncr = xIncr + yIncr ;
    float C = x0*y1 - x1*y0 ;

    int y_fb = (int) y_start;
    float d = yIncr*(x0+1.0f) + xIncr*(y0+.5f) + C ;
    int area = 0;
    //debug("d: %f", d);
    for (int x=x0; x<=x1; ++x) {
        // Calculate number of subpixels contained by line.

        debug("area: %i", area);
        if (d < 0) {
            //debug("d1: %f", d);
            d += bigIncr ;
            y += 1 ;
        }
        else {
            //debug("d2: %f", d);
            d += yIncr ;
        }
        if (!dir == 1) {
            area += y;
        } else {
            area += 5 - y;
        }

    }
    float shading = 1 - (float) area / 25.0f;
    *(unzoom_fb+fb_offset(fb_y, fb_x, 0)) = shading;
    *(unzoom_fb+fb_offset(fb_y, fb_x, 1)) = shading;
    *(unzoom_fb+fb_offset(fb_y, fb_x, 2)) = shading;
}
/** Draw the line.
 */
void draw_line() {
    if (do_antialias) draw_line_antialiased() ;
    else draw_line_aliased() ;
}

//
// DISPLAY CALLBACK FUNCTIONS
//

/** Draw the background into the unzoomed framebuffer.
 */
void draw_unzoom_background() {

    // Unconditionally reallocate the framebuffer.
    if (unzoom_fb != NULL) free(unzoom_fb) ;
    debug("draw_unzoom_background():  allocating in-memory framebuffer") ;
    unzoom_fb = malloc(win_width*win_height*3*sizeof(GLfloat)) ;

    // Corners of the zoom rectangle.
    zoom_llx = (win_width-ZOOM_RECT_WIDTH)/2 ;
    zoom_lly = (win_height-ZOOM_RECT_HEIGHT)/2 ;
    zoom_urx = zoom_llx + ZOOM_RECT_WIDTH -1 ;
    zoom_ury = zoom_lly + ZOOM_RECT_HEIGHT - 1 ;

    // Set the background to white.
    for (int i=0; i<win_width*win_height*3; ++i) *(unzoom_fb+i) = 1.0f ;

    // Draw a blue zoom rectangle.
    for (int i=zoom_llx; i<=zoom_urx; ++i) {
        *(unzoom_fb+fb_offset(zoom_lly, i, 0)) = 0.0f ;
        *(unzoom_fb+fb_offset(zoom_lly, i, 1)) = 0.0f ;
        *(unzoom_fb+fb_offset(zoom_ury, i, 0)) = 0.0f ;
        *(unzoom_fb+fb_offset(zoom_ury, i, 1)) = 0.0f ;
    }
    for (int i=zoom_lly; i<=zoom_ury; ++i) {
        *(unzoom_fb+fb_offset(i, zoom_llx, 0)) = 0.0f ;
        *(unzoom_fb+fb_offset(i, zoom_llx, 1)) = 0.0f ;
        *(unzoom_fb+fb_offset(i, zoom_urx, 0)) = 0.0f ;
        *(unzoom_fb+fb_offset(i, zoom_urx, 1)) = 0.0f ;
    }
}

/** Draw the unzoomed framebuffer.
 */
void draw_unzoom_fb() {
    draw_unzoom_background() ;
    draw_line() ;
}

/** Draw the zoomed framebuffer.
 */
void draw_zoom_fb() {
    if (zoom_fb != NULL) free(zoom_fb) ;
    zoom_fb = malloc(win_width*win_height*3*sizeof(GL_FLOAT)) ;

    // Set the background to white.
    for (int i=0; i<win_width*win_height*3; ++i) *(zoom_fb+i) = 1.0f ;

    int llx = win_width/2 - ZOOM_RECT_WIDTH*ZOOM_FACTOR/2 ;
    int lly = win_height/2 - ZOOM_RECT_HEIGHT*ZOOM_FACTOR/2 ;
    
    for (int x=0; x<ZOOM_RECT_WIDTH; ++x) {
        for (int y=0; y<ZOOM_RECT_HEIGHT; ++y) {
            for (int i=0; i<ZOOM_FACTOR; ++i) {
                for (int j=0; j<ZOOM_FACTOR; ++j) {
                    for (int c=0; c<3; ++c)
                        *(zoom_fb+fb_offset(lly+ZOOM_FACTOR*y+j,
                                    llx+ZOOM_FACTOR*x+i, c)) = 
                            *(unzoom_fb+fb_offset(zoom_lly+y, zoom_llx+x, c)) ;
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
    glWindowPos2s(0, 0) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_FLOAT, fb) ;
    glFlush() ;
    glutSwapBuffers() ;
}

/** Display the unzommed framebuffer.
 */
void display_unzoom() {
    draw_pixels(unzoom_fb) ;
}

/** Display the zoomed framebuffer.
 */
void display_zoom() {
    draw_pixels(zoom_fb) ;
}

/**
 * Get the fractional part of a float
 * @param value - a float
 */
float frac_part(float value) {
    return value - int_part(value);
}

/**
 * Get the integer part of a float
 * @param value - a float
 */
int int_part(float value) {
    return (int) value;
}
