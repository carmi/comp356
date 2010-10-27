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
#include "surface.h"
#include <float.h>

#include "debug.h"

#define DEFAULT_WIN_WIDTH 800
#define DEFAULT_WIN_HEIGHT 600

#define MALLOC1(t) (t *)(malloc(sizeof(t)))
#ifndef max
    #define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


// Window dimensions.
int win_width, win_height;

// GLUT Function Declarations
void draw_image(void);
void no_display(void);
void handle_reshape(int w, int h);

// Function Declarations
void set_camera_frame(point3_t* e, point3_t* P, vector3_t* up);
void get_dir_vec(int i, int j, vector3_t* result);
float* fb_offset(int x, int y, int c);
void add_two_colors(color_t* a, color_t* b, color_t* sum);
void mult_two_colors(color_t* a, color_t* b, color_t* product);

// Window identifiers.
int main_win;    // Main top-level window.

// Framebuffer identifier
float* fb;

// Surfaces and Lights identifiers
list356_t* surfaces;
list356_t* lights;

// View Data identifiers
point3_t* rt_eye;
point3_t* rt_look_at_point;
vector3_t* rt_up_dir;

// View Plane values
float rt_view_plane_dist;
float rt_view_plane_width;
float rt_view_plane_height;

// Camera Frame Basis
vector3_t* rt_u;
vector3_t* rt_v;
vector3_t* rt_w;

int main(int argc, char **argv) {
    debug("main(): get surfaces and lights") ;
    // Get surfaces and lights from surfaces_lights.c
    surfaces = get_surfaces();
    lights = get_lights();
    
    // Set view_data and view_plane
    rt_eye = MALLOC1(point3_t);
    rt_look_at_point = MALLOC1(point3_t);
    rt_up_dir = MALLOC1(vector3_t);
    set_view_data(rt_eye, rt_look_at_point, rt_up_dir);
    set_view_plane(&rt_view_plane_dist, &rt_view_plane_width, &rt_view_plane_height);
    
    // Calculate camera frame
    rt_u = MALLOC1(vector3_t);
    rt_v = MALLOC1(vector3_t);
    rt_w = MALLOC1(vector3_t);
    set_camera_frame(rt_eye, rt_look_at_point, rt_up_dir);
    
    // Initialize the drawing window.
    debug("main(): initialize main window.") ;
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT);
    glutInitWindowPosition(0, 0);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    
    // Create the main window, set the display callback.
    debug("main(): create main window.") ;
    main_win = glutCreateWindow("A Ray Tracer");
    // glutDisplayFunc(no_display);
    glutDisplayFunc(draw_image);
    glutReshapeFunc(handle_reshape);
    
    // Enter the main event loop.
    debug("main(): enter main loop.") ;
    glutMainLoop();
    
    free(rt_w); free(rt_v); free(rt_u);
    free(rt_up_dir); free(rt_look_at_point); free(rt_eye);
    lst_free(surfaces); lst_free(lights);
    return EXIT_SUCCESS;
}

/**
 * Display callback that iterates through framebuffer and ray traces.
 */
void draw_image() {
    for (int c = 0; c < win_width; c++) {
        for (int r = 0; r < win_height; r++) {
            // Create ray.
            ray3_t current_ray;
            current_ray.base = *rt_eye;
            vector3_t rayDir;
            get_dir_vec(c, r, &rayDir);
            current_ray.dir = rayDir;
            
            float t0 = 1;
            float t1 = FLT_MAX;
            
            hit_record_t rec;
            hit_record_t srec;
            
            list356_itr_t *surfaces_itr = lst_iterator(surfaces);
            bool hit_something = false;
            while (lst_has_next(surfaces_itr)) {
                surface_t *current_surface = lst_next(surfaces_itr);
                if (sfc_hit(current_surface, &current_ray, t0, t1, &rec)) {
                    if (rec.t < t1) {
                        hit_something = true;
                        srec.sfc = rec.sfc;
                        srec.t = rec.t;
                        srec.hit_pt = rec.hit_pt;
                        srec.normal = rec.normal;
                        t1 = rec.t;
                    }
                }
            }
            
            if (hit_something) {
                surface_t *closest_surface = srec.sfc;
                color_t *diffuse_color = closest_surface->diffuse_color;
                color_t *ambient_color = closest_surface->ambient_color;
                color_t *specular_color = closest_surface->spec_color;
                color_t *reflection_color = closest_surface->refl_color;
                float phong_exp = closest_surface->phong_exp;

                float t = srec.t;
                
                point3_t intersection_point = srec.hit_pt;

                float red_light_iten = 0.0f;
                float green_light_iten = 0.0f;
                float blue_light_iten = 0.0f;
                // Iterate through lights
                list356_itr_t *lights_itr = lst_iterator(lights);
                while (lst_has_next(lights_itr)) {
                    light_t* cur_light = lst_next(lights_itr);
                    point3_t* light_position = cur_light->position;

                    // Incidence angle is position of light minus intersection
                    // point
                    vector3_t incidence_angle;                  
                    pv_subtract(light_position, &intersection_point, &incidence_angle);
                    normalize(&incidence_angle);
                    
                    color_t* light_color = cur_light->color;
                    
                    // Calculate light intensities for each of RGB
                    // red
                    red_light_iten += (diffuse_color->red * light_color->red *
                            max(0, dot(&incidence_angle, &srec.normal)) );
                    green_light_iten += (diffuse_color->green * light_color->green *
                            max(0, dot(&incidence_angle, &srec.normal)) );
                    blue_light_iten += (diffuse_color->blue * light_color->blue *
                            max(0, dot(&incidence_angle, &srec.normal)) );
                    }
            
                
                // Calculate as in 4.5.4.
                
                // red
                *fb_offset(c, r, 0) = red_light_iten*diffuse_color->red;
                // green
                *fb_offset(c, r, 1) = green_light_iten*diffuse_color->green;
                // blue
                *fb_offset(c, r, 2) = blue_light_iten*diffuse_color->blue;
            }
            
            lst_iterator_free(surfaces_itr);
        }
    }
    debug("Done iterating through pixels.");
    glWindowPos2s(0, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(win_width, win_height, GL_RGB, GL_FLOAT, fb);
    
    glFlush();
    
    glutSwapBuffers();
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
    debug("handle_reshape(%d, %d)", w, h);
    win_width = w;
    win_height = h;

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
    pv_subtract(e, P, rt_w);
    normalize(rt_w);

    // Calculate u
    cross(up, rt_w, rt_u);
    normalize(rt_u);

    // Calculate v
    cross(rt_w, rt_u, rt_v);
}


/**
 * Map a screen pixel (i, j) to a point (in the world frame) on our view
 * rectangle/plane and calculate the vector from our viewpoint to this point.
 *
 * @param i - the column of the pixel.
 * @param j - the row of the pixel.
 * @param result - pointer to vector that will be assigned the result in world
 *                 frame.
 */
void get_dir_vec(int i, int j, vector3_t* result) {
    float x, y, z;
    x = (-rt_view_plane_width/2) + ( (((float)i + 0.5)/win_width)*rt_view_plane_width);
    y = (-rt_view_plane_height/2) + ( (((float)j + 0.5)/win_height)*rt_view_plane_height);
    z = -rt_view_plane_dist;
    
    // result is vector d in Shirley and Marschner notation.
    // Perform a change of basis on the vector result from camera frame to
    // world frame.
    vector3_t tmp_x, tmp_y, tmp_z;
    multiply(rt_u, x, &tmp_x);
    multiply(rt_v, y, &tmp_y);
    multiply(rt_w, z, &tmp_z);
    // Now sum tmp_x, tmp_y, tmp_z
    add(&tmp_x, &tmp_y, &tmp_x);
    add(&tmp_x, &tmp_z, result);
}

// Returns the pointer into the framebuffer that corresponds
// to pixel position (x, y) and color c.
float* fb_offset(int x, int y, int c) {
    return &fb[(y * win_width + x) * 3 + (c)];
}

// Adds color a and color b into color sum.
void add_two_colors(color_t* a, color_t* b, color_t* sum) {
    sum->red = a->red + b->red;
    sum->green = a->green + b->green;
    sum->blue = a->blue + b->blue;
}

// Multiplies color a and color b into color product.
void mult_two_colors(color_t* a, color_t* b, color_t* product) {
    product->red = a->red * b->red;
    product->green = a->green * b->green;
    product->blue = a->blue * b->blue;
}
