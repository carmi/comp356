/** Functions to load objects and lights.
 *
 */

#include <stdlib.h>
#include "list356.h"
#include "geom356.h"

#include "debug.h"

#include "color.h"
#include "surface.h"

// Camera frame data.
point3_t eye_position = {4.0f, 4.0f, 4.0f} ;
point3_t look_at_point = {-1.0f, -1.0f, -1.0f} ;
vector3_t up_dir = {0.0f, 1.0f, 0.0f} ;

// View-plane specification.
float view_plane_dist = 4.0f ;
float view_plane_width = 8.0f ;
float view_plane_height = 6.0f ;

// Colors.
color_t GREEN = {0.0f, 1.0f, 0.0f} ;
color_t WHITE = {1.0f, 1.0f, 1.0f} ;
color_t LIGHT_GREY = {.6f, .6f, .6f} ;
color_t DARK_GREY = {.2f, .2f, .2f} ;
color_t BLACK = {0.0f, 0.0f, 0.0f} ;
color_t PURPLE = {.5f, .2f, .5f} ;

/** Create a list of surfaces.
 */
list356_t* get_surfaces() {
    debug("get_surfaces()") ;


    // Spheres along the negative x, y, and z axes.
    list356_t* surfaces = make_list() ;
    surface_t* z_sphere ;
    for (float x=0.0; x>=-40.0; x-=3.0) {
        lst_add(surfaces, make_sphere(x, 0.0, 0.0f, 1.0f, &GREEN, 
                    &GREEN, &WHITE, 100.0f)) ;
        lst_add(surfaces, make_sphere(0.0f, x, 0.0f, .25, &PURPLE, 
                    &PURPLE, &WHITE, 100.0f)) ;
        lst_add(surfaces, (z_sphere = make_sphere(0.0f, 0.0f, x, .25, &PURPLE, 
                    &PURPLE, &WHITE, 100.0f))) ;
        z_sphere->refl_color = &LIGHT_GREY ;
    }

    // A rectangle in the y=-1 plane.
    surface_t* tri1 = make_triangle(
                (point3_t){-40, -1, 2},
                (point3_t){2, -1, 2},
                (point3_t){2, -1, -20},
                &LIGHT_GREY, &LIGHT_GREY, &BLACK, 10.0f) ;
    surface_t* tri2 = make_triangle(
                (point3_t){-40, -1, 2},
                (point3_t){2, -1, -20},
                (point3_t){-40, -1, -20},
                &LIGHT_GREY, &LIGHT_GREY, &BLACK, 10.0f) ;
    tri1->refl_color = &DARK_GREY ;
    tri2->refl_color = &DARK_GREY ;
    lst_add(surfaces, tri1) ;
    lst_add(surfaces, tri2) ;

    return surfaces ;

}

/** Create a list of lights.
 */
list356_t* get_lights() {
    list356_t* lights = make_list() ;
    light_t* l = malloc(sizeof(light_t)) ;
    l->position = malloc(sizeof(point3_t)) ;
    *(l->position) = (point3_t){50.0f, 50.0f, -50.0f} ;
    l->color = malloc(sizeof(color_t)) ;
    *(l->color) = (color_t){.75f, .75f, .75f} ;
    lst_add(lights, l) ;

    return lights ;
}

void set_view_data(point3_t* eye, point3_t* look_at, vector3_t* up) {
    *eye = eye_position ;
    *look_at = look_at_point ;
    *up = up_dir ;
}

void set_view_plane(float* dist, float* width, float* height) {
    *dist = view_plane_dist ;
    *width = view_plane_width ;
    *height = view_plane_height ;
}
