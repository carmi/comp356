/** Advanced ray-tracer implementation with transparency and bounding box trees.
 *
 *  @file final.c
 *  Professor Danner
 *  Computer Graphics 356
 *  Final Project
 *  Evan Carmi (WesID: 807136)
 *  ecarmi@wesleyan.edu
 *
 * ----CHANGES----
 *
 * Changes to this file from original HW2 Solution Ray Tracer:
 * Added the following new functions:
 * get_transparency()
 * refract()
 * reflect()
 *
 * Slightly modified the following functions:
 * get_specular_refl() - added in_trans parameter.
 * ray_trace() - added in_trans parameter and modified shadows for transparent
 *               objects.
 *
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
#include <stdarg.h>
#include <time.h>
#endif

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "list356.h"
#include "geom356.h"

#include "surface.h"
#include "surfaces_lights.h"

#include "debug.h"

#define max(a, b) a < b ? b : a

#define EPSILON .001

// Application data

// Window data.
const int DEFAULT_WIN_WIDTH = 800 ;
const int DEFAULT_WIN_HEIGHT = 600 ;
int win_width ;
int win_height ;

// Viewing data.
point3_t eye ;
point3_t look_at ;
vector3_t up_dir ;

// View-plane specification in camera frame basis.
float view_plane_dist ;
float view_plane_width ;
float view_plane_height ;

vector3_t eye_frame_u, eye_frame_v, eye_frame_w ;

// Surface data.
list356_t* surfaces = NULL ;

// Light data.
list356_t* lights = NULL ;
color_t ambient_light = {.1f, .1f, .1f} ;

// Callbacks.
void handle_display(void) ;
void handle_resize(int, int) ;

// Application functions.
void win2world(int, int, vector3_t*) ;
void compute_eye_frame_basis() ;
color_t get_transparency(ray3_t* ray, hit_record_t* hit_rec, int depth,
        bool in_trans);
bool refract(ray3_t* ray, vector3_t* normal, float refr_index,
        vector3_t* t_vec, bool in_trans);
void reflect(ray3_t* i_ray, vector3_t* normal, vector3_t* r_vec);

// Lighting functions.
color_t get_specular_refl(ray3_t* ray, hit_record_t* hit_rec, int depth, bool
        in_trans);
float get_lambert_scale(vector3_t* light_dir, hit_record_t* hit_rec) ;
float get_blinn_phong_scale(ray3_t* ray, vector3_t* light_dir, 
        hit_record_t* hit_rec) ;
void add_scaled_color(color_t* color, color_t* sfc_color, color_t*
        light_color, float scale) ;

// The in-memory copy of the framebuffer; allocated by handle_resize.
GLfloat* fb ;

void handle_exit() ;

int main(int argc, char **argv) {

    // Initialize the drawing window.
    glutInitWindowSize(DEFAULT_WIN_WIDTH, DEFAULT_WIN_HEIGHT) ;
    glutInitWindowPosition(0, 0) ;
    glutInit(&argc, argv) ;
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB) ;

    // Create the main window.
    glutCreateWindow("Ray tracer") ;
    glutReshapeFunc(handle_resize) ;
    glutDisplayFunc(handle_display) ;

    // Application initialization.
    set_view_data(&eye, &look_at, &up_dir) ;
    set_view_plane(&view_plane_dist, &view_plane_width, &view_plane_height) ;
    surfaces = get_surfaces() ;
    lights = get_lights() ;
    compute_eye_frame_basis() ;

    // Enter the main event loop.
    atexit(handle_exit) ;
    glutMainLoop() ;

    return EXIT_SUCCESS ;
}

void handle_exit() {
    debug("handle_exit()") ;
    if (fb != NULL) free(fb) ;
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

    if (fb != NULL) free(fb) ;
    debug("handle_resize():  allocating in-memory framebuffer") ;
    fb = malloc(win_width*win_height*3*sizeof(GLfloat)) ;
    bzero(fb, (win_width*win_height*3)*sizeof(GLfloat)) ;

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

/** Get the shade determined by a given ray.
 *  
 *  @param ray the ray to trace.
 *  @param t0 the start of the interval for which to get a shade.
 *  @param t1 the end of the interval for which to get a shade.
 *  @param depth the maximum number of times a reflect ray will be
 *      cast for objects with non-NULL reflective color.
 *  @param in_trans whether the ray is inside a transparent surface or not.
 * 
 *
 *  @return the color corresponding to the closest object hit by
 *      <code>r</code> in the interval <code>[t0, t1]</code>.
 */
color_t ray_trace(ray3_t ray, float t0, float t1, int depth, bool in_trans) {
    assert(depth >= 0) ;

    color_t color = {0.0, 0.0, 0.0} ;

    if (depth ==0) return color ;

    hit_record_t hit_rec, closest_hit_rec ;

    // Get a hit record for the closest object that is hit.
    bool hit_something = false ;
    list356_itr_t* s = lst_iterator(surfaces) ;
    while (lst_has_next(s)) {
        surface_t* sfc = lst_next(s) ;
        if (sfc_hit(sfc, &ray, t0, t1, &hit_rec)) {
            if (hit_rec.t < t1) {
                hit_something = true ;
                memcpy(&closest_hit_rec, &hit_rec, 
                        sizeof(hit_record_t)) ;
                t1 = hit_rec.t ;
            }
        }
    }
    lst_iterator_free(s) ;

    // If we hit something, color the pixel.
    if (hit_something) {
        surface_t* sfc = closest_hit_rec.sfc ;

        // Specular reflection.
        if (sfc->refl_color != NULL) {
            color_t refl_color = get_specular_refl(&ray,
                    &closest_hit_rec, depth, in_trans) ;
            add_scaled_color(&color, sfc->refl_color, &refl_color, 1.0f) ;
        }

        // Tranparency
        if (sfc->refr_index != -1) {
            color_t trans_color = get_transparency(&ray, &closest_hit_rec,
                    depth, !in_trans);
            // Only add returned color, don't multiply by a surface_color.
            color.red += (trans_color.red);
            color.green += (trans_color.green);
            color.blue += (trans_color.blue);
        }
        // Ambient shading.
        add_scaled_color(&color, sfc->ambient_color, &ambient_light, 1.0f) ;

        // Lighting.
        list356_itr_t* light_itr = lst_iterator(lights) ;
        while (lst_has_next(light_itr)) {
            light_t* light = lst_next(light_itr) ;
            vector3_t light_dir ;
            pv_subtract(light->position, &(closest_hit_rec.hit_pt),
                    &light_dir) ;
            normalize(&light_dir) ;

            // Check for global shadows.
            bool do_lighting = true ;

            // Bool for if the shadow is caused by transparent surface.
            bool trans_shadow = false ;

            ray3_t light_ray = {closest_hit_rec.hit_pt, light_dir} ;
            float light_dist = dist(&closest_hit_rec.hit_pt,
                    light->position) ;
            s = lst_iterator(surfaces) ;
            while (lst_has_next(s)) {
                surface_t* sfc = lst_next(s) ;
                if (sfc_hit(sfc, &light_ray, EPSILON, light_dist, &hit_rec)) {
                    do_lighting = false ;
                    // If shadow is caused by transparent surface, we add
                    // Lambertian shading only so our shadows are not opaque.
                    if (hit_rec.sfc->refr_index != -1) trans_shadow = true;
                    break ;
                }
            }
            lst_iterator_free(s) ;
            if (!do_lighting) {
                if (!trans_shadow) continue;
            }

            // Lambertian shading.
            float scale = get_lambert_scale(&light_dir, &closest_hit_rec) ;
            add_scaled_color(&color, sfc->diffuse_color, light->color,
                    scale) ;
            //}

            // Blin-Phong shading (if shadow is not caused by transparent
            // surface).
            if (!trans_shadow) {
                float phong_scale = get_blinn_phong_scale(&ray, &light_dir,
                        &closest_hit_rec) ;
                add_scaled_color(&color, sfc->spec_color, light->color, 
                        phong_scale) ;
            }
        }   // while(lst_has_next(light_itr))

        lst_iterator_free(light_itr) ;

    }   // if (hit_something)
    return color ;
}

/** Get the shade from specular reflection.
 *  
 * @param ray the viewing ray.
 * @param hit_rec the hit record for the point being shaded.
 * @param depth the current ray-tracing recursion depth.
 * @param in_trans whether the ray is inside a transparent surface or not.
 *
 *  @return the color to add from ideal specular reflection of <code>ray</code>.
 */
color_t get_specular_refl(ray3_t* ray, hit_record_t* hit_rec, int depth, bool
        in_trans) {
    ray3_t refl_ray ;
    refl_ray.base = hit_rec->hit_pt ;
    refl_ray.dir = hit_rec->normal ;
    multiply(&hit_rec->normal, 
            2*dot(&ray->dir, &hit_rec->normal), 
            &refl_ray.dir) ;
    subtract(&ray->dir, &refl_ray.dir, &refl_ray.dir) ;
    color_t refl_color = ray_trace(refl_ray, EPSILON, FLT_MAX, 
            depth-1, in_trans) ;
    return refl_color ;
}

/**
 * Get the color from reflection and refraction for transparent objects.
 *
 * @param ray the viewing ray
 * @param hit_rec the hit record for the point being colored.
 * @param depth the current ray-tracing recursion depth.
 * @param in_trans whether the ray is inside a transparent surface or not.
 *
 * @return the color to add from transparency of ray.
 */
color_t get_transparency(ray3_t* ray, hit_record_t* hit_rec, int depth, bool
        in_trans) {
    //debug("get_transparency");

    // Use notation from Shirley and Marschner:
    // d - ray
    // n - normal = closest_hit_rec->normal
    // <i>n></i> - refractive index
    // t - transformed ray direction

    // Calculate reflected ray refl_ray: r = reflect(d,n).
    vector3_t normal = hit_rec->normal;
    vector3_t refl_vector;
    reflect(ray, &normal, &refl_vector);
    normalize(&refl_vector);
    ray3_t refl_ray = { hit_rec->hit_pt, refl_vector };

    // Store locally for cleaner code.
    surface_t* sfc = hit_rec->sfc;
    float index = sfc->refr_index;

    // Variables populated in if/else blocks:
    color_t trans_color;
    color_t k;
    float c;
    vector3_t t_vec;

    // Normalize ray_dir for dot product
    vector3_t ray_dir = ray->dir;
    normalize(&ray_dir);
    if (dot(&ray_dir, &normal) < 0) {
        // Calculate direction refracted (transformed) ray, t_vec; 
        refract(ray, &normal, index, &t_vec, in_trans);
        c = ( -1.0f * dot(&ray_dir, &normal));
        k = (color_t) {1.0f, 1.0f, 1.0f};
    } else {

        // Intensity of light diminishes by the attenuation constant of the
        // surface proportionally to the distance the ray of light travels
        // through the surface. To calculate this distance that the ray of
        // light spends in the surface we follow a procedure similar to the
        // procedure of shadows. We Iterate through all surfaces looking for
        // the closest hit object in the direction of the refracted ray. Then
        // we take the distance between the first hit records hit_pt and the
        // closest hit record's hit_pt which is on the closest object.

        vector3_t dist_vec;
        refract(ray, &normal, index, &dist_vec, in_trans);
        ray3_t t_ray = {hit_rec->hit_pt, dist_vec};
        hit_record_t t_hit_rec, t_closest_hit_rec ;

        // Get a hit record for the closest object that is hit in dir t_ray.
        bool hit_something = false ;
        float t1 = FLT_MAX;
        list356_itr_t* s = lst_iterator(surfaces) ;
        while (lst_has_next(s)) {
            surface_t* sfc = lst_next(s) ;
            if (sfc_hit(sfc, &t_ray, EPSILON, t1, &t_hit_rec)) {
                if (t_hit_rec.t < t1) {
                    hit_something = true ;
                    memcpy(&t_closest_hit_rec, &t_hit_rec, 
                            sizeof(hit_record_t)) ;
                    t1 = t_hit_rec.t ;
                }
            }
        }
        lst_iterator_free(s) ;
        float t = dist(&hit_rec->hit_pt, &t_closest_hit_rec.hit_pt);

        // Calculate attenuation.
        color_t* a = sfc->atten;
        k = (color_t) {
            exp(-1.0f * (a->red) * t),
            exp(-1.0f * (a->green) * t),
            exp(-1.0f * (a->blue) * t) };

        vector3_t neg_normal; // neg_normal = -n
        multiply(&normal, -1.0f, &neg_normal);
        // inv_index = 1/n = 1/refr_index
        float inv_index = 1.0f/index;
        if (refract(ray, &neg_normal, inv_index, &t_vec, in_trans)) {
            c = dot(&t_vec, &normal);
        } else {
            trans_color = ray_trace(refl_ray, EPSILON, FLT_MAX, depth-1,
                    !in_trans);
            trans_color.red = trans_color.red*k.red;
            trans_color.green = trans_color.green*k.green;
            trans_color.blue = trans_color.blue*k.blue;
            //return (color_t) {1.0f, 1.0f, 0.0f};
            return trans_color;
        }
    }

    float R0 = (( (index - 1)*(index - 1) )/( (index + 1)*(index + 1) ));
    float R = (R0 + (1 - R0)*((1 - c)*(1 - c)*(1 - c)*(1 - c)*(1 - c)));
    color_t trans_color1, trans_color2;

    // Recursively ray trace on the reflected ray and the refracted ray.
    trans_color1 = ray_trace(refl_ray, EPSILON, FLT_MAX, depth-1, !in_trans);
    ray3_t t_ray = {hit_rec->hit_pt, t_vec};
    trans_color2 = ray_trace(t_ray, EPSILON, FLT_MAX, depth-1, !in_trans);

    // Combine the reflected and refracted ray and return.
    trans_color.red = k.red*( (R*trans_color1.red) + ((1.0f -
                    R)*trans_color2.red));
    trans_color.green = k.green*( (R*trans_color1.green) + ((1.0f -
                    R)*trans_color2.green));
    trans_color.blue = k.blue*( (R*trans_color1.blue) + ((1.0f -
                    R)*trans_color2.blue));
    return trans_color;
}

/** Get the scale factor for Lambertian (diffuse) shading from a single
 *  light source.
 *  
 *  @param light_dir the direction to the light.
 *  @param hit_rec the hit record for the point being shaded.
 *
 *  @return the scale factor to use for Lambertian shading.
 */
float get_lambert_scale(vector3_t* light_dir, hit_record_t* hit_rec) {
    return max(0.0f, dot(light_dir, &(hit_rec->normal))) ;
}

/** Get the scale factor for Blinn-Phong specular highlighting from a
 *  single light source.
 *
 *  @param ray the viewing ray.
 *  @param light_dir the direction to the light.
 *  @param hit_rec the hit record for the point being shaded.
 *
 *  @return the scale factor to use for Blinn-Phong shading.
 */
float get_blinn_phong_scale(ray3_t* ray, vector3_t* light_dir, 
        hit_record_t* hit_rec) {
    vector3_t view, half_v ;
    multiply(&ray->dir, -1.0, &view) ;
    normalize(&view) ;
    add(&view, light_dir, &half_v) ;
    normalize(&half_v) ;
    float phong_scale = max(0, 
            dot(&half_v, &hit_rec->normal)) ;
    phong_scale = pow(phong_scale, hit_rec->sfc->phong_exp) ;
    return phong_scale ;
}

/** Add a scaled product of surface and light colors to a given color.
 *  Calling this function has the effect of executing
 *  <pre>
 *      color->C += (surface_color->C)*(light_color->C)*scale
 *  </pre>
 *  for each color component <code>C</code>.
 *  
 *  @param color the base color.
 *  @param sfc_color the surface color.
 *  @param light_color the light color.
 *  @param scale the amount to scale the additional color by.
 */
void add_scaled_color(color_t* color, color_t* sfc_color, color_t* light_color,
        float scale) {
    color->red += (sfc_color->red)*(light_color->red)*scale ;
    color->green += (sfc_color->green)*(light_color->green)*scale ;
    color->blue += (sfc_color->blue)*(light_color->blue)*scale ;
}

/** Display callback; render the scene.
 */
void handle_display() {
    // The ray itself.
    ray3_t ray ;
    ray.base = eye ;

    color_t color ;

#ifndef NDEBUG
    clock_t start_time, end_time ;
    start_time = clock() ;
#endif
    for (int x=0; x<win_width; ++x) {
        for (int y=0; y<win_height; ++y) {
            win2world(x, y, &ray.dir) ;
            debug_c((x==400 && y==300),
                "view ray = {(%f, %f, %f), (%f, %f, %f)}.\n",
                eye.x, eye.y, eye.z, 
                ray.dir.x, ray.dir.y, ray.dir.z) ;
            //Start ray eye assuming we're not inside a transparent surface.
            color = ray_trace(ray, 1.0 + EPSILON, FLT_MAX, 5, false) ;
            *(fb+fb_offset(y, x, 0)) = color.red ;
            *(fb+fb_offset(y, x, 1)) = color.green ;
            *(fb+fb_offset(y, x, 2)) = color.blue ;
        }
    }
#ifndef NDEBUG
    end_time = clock() ;
    debug("handle_display(): frame calculation time = %f sec.",
            ((double)(end_time-start_time))/CLOCKS_PER_SEC) ;
#endif

    // The following line throws a implicit declaration compiler warning: but
    // it was in hw2bp1.c solution file so I will ignore it.
    glWindowPos2s(0, 0) ;
    glDrawPixels(win_width, win_height, GL_RGB, GL_FLOAT, fb) ;
    glFlush() ;
    glutSwapBuffers() ;
}

/** Compute the eye frame basis from the eye point, look-at point,
 *  and up direction.  The basic algorithm:
 *  -# w <- normalized (eye - look_at)
 *  -# u <- normalized (up x w)
 *  -# v <- w x u.
 */
void compute_eye_frame_basis() {
    pv_subtract(&eye, &look_at, &eye_frame_w) ;
    normalize(&eye_frame_w) ;

    cross(&up_dir, &eye_frame_w, &eye_frame_u) ;
    normalize(&eye_frame_u) ;

    cross(&eye_frame_w, &eye_frame_u, &eye_frame_v) ;

    debug("compute_eye_frame_basis():  eye_frame_u = (%f, %f, %f)",
            eye_frame_u.x, eye_frame_u.y, eye_frame_u.z) ;
    debug("compute_eye_frame_basis():  eye_frame_v = (%f, %f, %f)",
            eye_frame_v.x, eye_frame_v.y, eye_frame_v.z) ;
    debug("compute_eye_frame_basis():  eye_frame_w = (%f, %f, %f)",
            eye_frame_w.x, eye_frame_w.y, eye_frame_w.z) ;
}

/** Compute the viewing ray direction in the world frame basis.
 *
 *  @param x the x-position on the window (in pixels), starting at the left.
 *  @param y the y-position on the window (in pixels), starting at the top.
 *  @param dir a vector3_t object that will be filled with the coordinates
 *      (in the world frame basis) for the direction of the viewing ray 
 *      through <code>(x, y)</code>.
 */
void win2world(int x, int y, vector3_t* dir) {
    // Compute coordinates in eye frame of corners of view plane.
    float left = -view_plane_width/2.0f ;
    float bottom = -view_plane_height/2.0f ;

    // Compute vector from eye to window position in eye coordinates.
    float u = left + (x+.5f)/win_width*view_plane_width ;
    float v = bottom + (y+.5f)/win_height*view_plane_height ;
    float w = -view_plane_dist ;
    debug_c((x == 400 && y == 300),
            "win2world():  u, v, w, = %f, %f, %f", u, v, w) ;

    // Transform vector to world coordinates.
    dir->x = u*eye_frame_u.x + v*eye_frame_v.x + w*eye_frame_w.x ;
    dir->y = u*eye_frame_u.y + v*eye_frame_v.y + w*eye_frame_w.y ;
    dir->z = u*eye_frame_u.z + v*eye_frame_v.z + w*eye_frame_w.z ;
    debug_c((x==400 && y==300),
        "win2world():  i, j, k = %f, %f, %f\n", dir->x, dir->y, dir->z) ;
}

/**
 * refract - Calculates the refracted ray and store it at t_ray. If there is
 * total internal reflection return false and do not store anything at t_ray,
 * otherwise return true.
 *
 * This assumes the other surface refr_index == 1.0 (like air) as we assume
 * all surfaces are surrounded by air. 
 *
 * @param ray - the ray that is hitting the surface
 * @param normal - the normal vector of the surface
 * @param refr_index - the refractive index of the surface
 * @param t_vec - location where to store the direction of transformed ray.
 */
bool refract(ray3_t* ray, vector3_t* normal, float refr_index, vector3_t*
        t_vec, bool in_trans) {
    // Notation from Shirley and Marschner page 305.
    // Calculate part 2 first, and check for total internal reflection.

    vector3_t* d = &ray->dir;
    vector3_t* n = normal;
    normalize(d);
    normalize(n);

    float d_dot_n = dot(d, n);
    float d_dot_n2 = d_dot_n * d_dot_n;

    // Refraction index ratio:
    float refr_ratio;
    // Assume objects are surrounded by air with index 1.0. As we move between
    // transparent and non transparent surfaces our ratio is either refr_index
    // or 1/refr_index.
    if (!in_trans) refr_ratio = refr_index;
    else refr_ratio = 1.0f/refr_index;

    //refr_ratio = 1.0f/refr_index;
    float refr_ratio2 = refr_ratio * refr_ratio;

    // Part 2: (1 - n^2(1 - d_dot_n^2)/1)
    float under_sqrt = (1.0f - refr_ratio2*(1 - d_dot_n2));
    if (under_sqrt < 0) {
        // Total internal reflection.
        return false;
    }

    vector3_t part2;
    multiply(n, sqrt(under_sqrt), &part2);

    // Part 1: n(d - n(d dot n)
    vector3_t ndn, d_minus_ndn, part1;
    // d - n(d (dot) n)
    multiply(n, d_dot_n, &ndn);
    subtract(d, &ndn, &d_minus_ndn);

    multiply(&d_minus_ndn, refr_ratio, &part1);

    // Store part1 - part2 at t_ray, return true.
    subtract(&part1, &part2, t_vec);
    normalize(t_vec);
    return true;
}

/**
 * Calculate the direction of a reflected ray off of a surface.
 *
 * @param i_ray - the incoming incident ray.
 * @param normal - the normal vector of the surface.
 * @param r_vec - the vector to store the reflected ray's direction at.
 */
void reflect(ray3_t* i_ray, vector3_t* normal, vector3_t* r_vec) {
    // Let v = incident vector, n = normal vector
    // Reflection vector = v - 2(v dot n)n
    vector3_t v = i_ray->dir;
    vector3_t tmp;
    
    // 2 * (v dot n) n
    multiply(normal, (2.0f*dot(&v, normal)), &tmp);

    subtract(&v, &tmp, r_vec);
}
