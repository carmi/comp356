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
#include <time.h>
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

// Epsilon value for shadows and reflections.
#define EPSILON 0.1

// Shading effects
#define USE_LAMBERT true
#define USE_BLINN_PHONG true

// Global effects
#define USE_AMBIENT_LIGHTING true
#define USE_SHADOWS true
#define USE_REFLECTIONS true

// Ambient Light
color_t ambient_light = {.1f, .1f, .1f};

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
void set_pixel_color(int x, int y, color_t* color);
void add_to_color(color_t* a, color_t* b);
void mult_two_colors(color_t* a, color_t* b, color_t* product);
void mult_color_coefficient(color_t* a, float b, color_t* result);
color_t* ray_color(ray3_t* current_ray, float t0, float t1, int depth);

// Window identifiers.
int main_win;    // Main top-level window.

// Framebuffer identifier
float* fb;

bool fb_in_memory;

// Global variables for measuring running time.
// clock_t clock_sum = 0;
// int clock_runs = 0;

// Surfaces and Lights identifiers
list356_t* rt_surfaces;
list356_t* rt_lights;

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
    rt_surfaces = get_surfaces();
    rt_lights = get_lights();
    
    // Set view_data and view_plane
    rt_eye = MALLOC1(point3_t);
    rt_look_at_point = MALLOC1(point3_t);
    rt_up_dir = MALLOC1(vector3_t);
    set_view_data(rt_eye, rt_look_at_point, rt_up_dir);
    set_view_plane(&rt_view_plane_dist, &rt_view_plane_width, \
                   &rt_view_plane_height);
    
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
    
    fb_in_memory = false;

    // Create the main window, set the display callback.
    debug("main(): create main window.") ;
    main_win = glutCreateWindow("A Ray Tracer");
    // glutDisplayFunc(no_display);
    glutDisplayFunc(draw_image);
    glutReshapeFunc(handle_reshape);
    
    // Enter the main event loop.
    debug("main(): enter main loop.") ;
    glutMainLoop();
    
    // Free malloc'd structures
    free(rt_w); free(rt_v); free(rt_u);
    free(rt_up_dir); free(rt_look_at_point); free(rt_eye);
    lst_free(rt_surfaces); lst_free(rt_lights);
    return EXIT_SUCCESS;
}

/**
 * Display callback that iterates through framebuffer and ray traces.
 */
void draw_image() {
    /* Parallelize ray tracing code if compiling with OpenMP. GCC 4.2+ can
     * compile with OpenMP using the -fopenmp switch. Set the environment
     * variable OMP_NUM_THREADS to specify the number of threads to use.
     * (Try setting the number of threads to the number of processors/cores.)
     */
    // debug("Start time: %d", start_time);
    // clock_t start_time = clock();
    #ifdef _OPENMP
        #pragma omp parallel for
    #endif
    for (int c = 0; c < win_width; c++) {
        for (int r = 0; r < win_height; r++) {
            // Create ray.
            ray3_t *current_ray = MALLOC1(ray3_t);
            current_ray->base = *rt_eye;
            vector3_t ray_dir;
            get_dir_vec(c, r, &ray_dir);
            current_ray->dir = ray_dir;
            
            color_t *pixel_color = ray_color(current_ray, 0, FLT_MAX, 0);

            // Set framebuffer pixels.
            *fb_offset(c, r, 0) = pixel_color->red;
            *fb_offset(c, r, 1) = pixel_color->green;
            *fb_offset(c, r, 2) = pixel_color->blue;

            free(pixel_color);
            free(current_ray);
        }
    }
    // Speed measurement
    // clock_t end_time = clock();
    //     //debug("End time: %d", end_time);
    //     clock_t time_delta = (end_time - start_time);
    //     debug("Time delta: %d", time_delta);
    //     clock_sum += time_delta;
    //     clock_runs += 1;
    //     clock_t average = (clock_sum / clock_runs );
    // debug("Finished drawing pixels.");
    // debug("Drawing took: %d", time_delta);
    // debug("Average took: %d", average);
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

    if (fb_in_memory) {
        free(fb);
        fb_in_memory = false;
    }

    // (Re)create framebuffer - array of floats
    fb = malloc(win_width * win_height * 3 * sizeof(float));
    fb_in_memory = true;
    //bzero(fb, (win_width*win_height*3)*sizeof(float));
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
    x = (-rt_view_plane_width/2) + ( (((float)i + 0.5)/win_width)*
            rt_view_plane_width);
    y = (-rt_view_plane_height/2) + ( (((float)j + 0.5)/win_height)*
            rt_view_plane_height);
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

/**
 * Get a pointer to the framebuffer that corresponds to pixel position (x, y)
 * and color c. Since we're specifying our color intensities as floats, this
 * pointer points to a float (which we can also explicitly set).
 *
 * @param x - the column of the pixel.
 * @param y - the row of the pixel.
 * @param c - which color of the pixel to access. 0 - red, 1 - green, 2 - blue.
 */
float* fb_offset(int x, int y, int c) {
    return &fb[(y * win_width + x) * 3 + (c)];
}

/**
 * Add one color into another.
 *
 * @param a - the color to be added into.
 * @param b - the color to add.
 */
void add_to_color(color_t* a, color_t* b) {
    a->red = a->red + b->red;
    a->green = a->green + b->green;
    a->blue = a->blue + b->blue;
}

/**
 * Multiply two colors together.
 *
 * @param a - a color to be multiplied.
 * @param b - a color to be multiplied.
 * @param product - the result of the multiplication.
 */
void mult_two_colors(color_t* a, color_t* b, color_t* product) {
    product->red = a->red * b->red;
    product->green = a->green * b->green;
    product->blue = a->blue * b->blue;
}

/**
 * Multiply a color by a coefficient.
 *
 * @param a - the color to be multiplied.
 * @param b - the floating point coefficient to be multiplied.
 * @param product - the result of the multiplication.
 */
void mult_color_coefficient(color_t* a, float b, color_t* product) {
    product->red = a->red * b;
    product->green = a->green * b;
    product->blue = a->blue * b;
}

/**
 * Run the ray tracing algorithm on a particular ray. We find the closest
 * surface that the ray hits and return the color of the intersection point,
 * applying any number of shading models.
 *
 * @param current_ray - the ray to trace.
 * @param t0 - the lower bound of the time interval of the algorithm. Used 
 *             by the surface hit functions to detect whether a surface
 *             has been hit.
 * @param t1 - the upper bound of the time interval of the algorithm. Used 
 *             by the surface hit functions to detect whether a surface
 *             has been hit.
 * @param depth - the recursion depth of the ray tracer (used for reflections).
 */
color_t* ray_color(ray3_t* current_ray, float t0, float t1, int depth) {
    color_t *pixel_color = MALLOC1(color_t);
    pixel_color->red = 0.0f;
    pixel_color->green = 0.0f;
    pixel_color->blue = 0.0f;
    hit_record_t rec;
    hit_record_t srec;
    list356_itr_t *surfaces_itr = lst_iterator(rt_surfaces);
    bool hit_something = false;
    while (lst_has_next(surfaces_itr)) {
        surface_t *current_surface = lst_next(surfaces_itr);
        if (sfc_hit(current_surface, current_ray, t0, t1, &rec)) {
            if (rec.t < t1) {
                hit_something = true;
                srec = rec;
                t1 = rec.t;
            }
        }
    } 
    lst_iterator_free(surfaces_itr);
    if (hit_something) {
        surface_t *closest_surface = srec.sfc;
        color_t *diffuse_color = closest_surface->diffuse_color;
        color_t *ambient_color = closest_surface->ambient_color;
        color_t *specular_color = closest_surface->spec_color;
        color_t *reflection_color = closest_surface->refl_color;
        float phong_exp = closest_surface->phong_exp;
        
        point3_t intersection_point = srec.hit_pt;

        // Begin with ambient lighting.
        if (USE_AMBIENT_LIGHTING) {
            mult_two_colors(ambient_color, &ambient_light, pixel_color);
        }

        // Iterate through lights
        // Calculate as in 4.5.4 in text.
        list356_itr_t *lights_itr = lst_iterator(rt_lights);
        while (lst_has_next(lights_itr)) {
            light_t* cur_light = lst_next(lights_itr);
            point3_t* light_position = cur_light->position;

            // light_dir is vector from intersection point to the
            // position of light.
            vector3_t light_dir;                  
            pv_subtract(light_position, &intersection_point, &light_dir);
            normalize(&light_dir);
            
            // light_intensity  = light intensity of the light, I in text.
            color_t* light_intensity = cur_light->color;
            
            // Using Shirley-Marschner notation, section 4.7.
            // construct a ray with base intersection_point, direction light_dir
            ray3_t p_plus_sl = {intersection_point, light_dir};
            // iterate through all surfaces
            hit_record_t shadow_rec;
            hit_record_t shadow_srec;
            float shadow_t0 = EPSILON;
            float shadow_t1 = FLT_MAX;
            list356_itr_t *shadow_surfaces_itr = lst_iterator(rt_surfaces);
            bool shadow_hit_something = false;
            while (lst_has_next(shadow_surfaces_itr)) {
                surface_t *shadow_current_surface =
                    lst_next(shadow_surfaces_itr);
                if (sfc_hit(shadow_current_surface, &p_plus_sl, shadow_t0,
                        shadow_t1, &shadow_rec)) {
                    if (shadow_rec.t < shadow_t1) {
                        shadow_hit_something = true;
                        shadow_srec = shadow_rec;
                        shadow_t1 = shadow_rec.t;
                    }
                }
            } 
            lst_iterator_free(shadow_surfaces_itr);
            
            color_t temp1, temp2;
            if (!shadow_hit_something || !USE_SHADOWS) {
                // Lambertian Diffuse Shading
                // Calculate diffuse_max_term once and re-use result.
                if (USE_LAMBERT) {
                    float diffuse_max_term = max(0, dot(&light_dir,
                        &srec.normal));

                    mult_two_colors(diffuse_color, light_intensity, &temp1);
                    mult_color_coefficient(&temp1, diffuse_max_term, &temp2);
                    add_to_color(pixel_color, &temp2);
                }

                // Blinn-Phong Shading
                if (USE_BLINN_PHONG) {
                    // Notation in Shirley and Marschner on pg. 83 is:
                    // h = half_vector = normalize(v + l)
                    // l = light_dir; v = view direction = normalize(-d)
                    vector3_t view_dir;
                    multiply(&current_ray->dir, -1.0f, &view_dir);
                    normalize(&view_dir);

                    vector3_t half_vec;
                    add(&view_dir, &light_dir, &half_vec);
                    normalize(&half_vec);
            
                    float bp_dot = dot(&srec.normal , &half_vec);
                    float bp_max_term = (double) pow( max(0,bp_dot),
                        (double) phong_exp);

                    mult_two_colors(specular_color, light_intensity, &temp1);
                    mult_color_coefficient(&temp1, bp_max_term, &temp2);
                    add_to_color(pixel_color, &temp2);
                }
            }
            // Calculate reflections
            if (reflection_color != NULL && depth < 5 && USE_REFLECTIONS) {
                // Notation from Shirley-Marschner 4.8.
                ray3_t reflection_ray;
                vector3_t twodnn, r;
                float two_d_dot_n = 2 * dot(&current_ray->dir, &srec.normal);
                multiply(&srec.normal, two_d_dot_n, &twodnn);
                subtract(&current_ray->dir, &twodnn, &r);
                normalize(&r);
                
                reflection_ray.base = intersection_point;
                reflection_ray.dir = r;
                
                color_t *reflection_ray_color = ray_color(&reflection_ray,
                    EPSILON, FLT_MAX, depth + 1);
                mult_two_colors(reflection_color, reflection_ray_color, &temp1);
                add_to_color(pixel_color, &temp1);
                free(reflection_ray_color);
            }
        }
        lst_iterator_free(lights_itr);
    }
    return pixel_color;
}
