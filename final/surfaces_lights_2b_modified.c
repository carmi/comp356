/** Functions to load objects and lights.
 *
 */

#include <stdlib.h>

#include "list356.h"

#include "color.h"
#include "surface.h"

// Camera frame data.
point3_t eye_position = {25.0f, 2.0f, 25.0f} ;
point3_t look_at_point = {-1.0f, -1.0f, -1.0f} ;
vector3_t up_dir = {0.0f, 1.0f, 0.0f} ;

// View-plane specification.
float view_plane_dist = 7.0f ;
float view_plane_width = 8.0f ;
float view_plane_height = 6.0f ;

// Colors.
color_t GREEN = {0.0f, 1.0f, 0.0f} ;
color_t WHITE = {1.0f, 1.0f, 1.0f} ;
color_t LIGHT_GREY = {.6f, .6f, .6f} ;
color_t DARK_GREY = {.2f, .2f, .2f} ;
color_t BLACK = {0.0f, 0.0f, 0.0f} ;
color_t PURPLE = {.5f, .2f, .5f} ;
color_t GOLD = {255.0f/255, 215.0f/255, 0.0f} ;

/** Create a list of surfaces.
 */
list356_t* get_surfaces() {

    list356_t* surfaces = make_list() ;

    // Plane at y=-1.
    surface_t* plane = make_plane(
                (point3_t){-40, -1, 2},
                (point3_t){2, -1, 2},
                (point3_t){2, -1, -20},
                &LIGHT_GREY, &LIGHT_GREY, &BLACK, 10.0f) ;
    plane->refl_color = &LIGHT_GREY ;
    lst_add(surfaces, plane) ;

    // Two walls.
    surface_t* w11 = make_triangle(
            (point3_t){-10, -1, -10},
            (point3_t){10, -1, -10},
            (point3_t){-10, 5, -10},
            &GOLD, &GOLD, &BLACK, 10.0f) ;
    surface_t* w12 = make_triangle(
            (point3_t){-10, 5, -10},
            (point3_t){10, -1, -10},
            (point3_t){10, 5, -10},
            &GOLD, &GOLD, &BLACK, 10.0f) ;
    lst_add(surfaces, w11) ;
    lst_add(surfaces, w12) ;

    surface_t* w21 = make_triangle(
            (point3_t){-10, -1, -10},
            (point3_t){-10, 5, -10},
            (point3_t){-10, 5, 10},
            &GOLD, &GOLD, &BLACK, 100.0f) ;
    surface_t* w22 = make_triangle(
            (point3_t){-10, -1, -10},
            (point3_t){-10, 5, 10},
            (point3_t){-10, -1, 10},
            &GOLD, &GOLD, &BLACK, 100.0f) ;
    lst_add(surfaces, w21) ;
    lst_add(surfaces, w22) ;

    surface_t* w23 = make_triangle(
            (point3_t){3, -1, -5},
            (point3_t){3, 5, 5},
            (point3_t){3, -1, 5},
            &GOLD, &GOLD, &BLACK, 100.0f) ;
    w23->refr_index = 1.33f;
    w23->atten=&LIGHT_GREY;
    lst_add(surfaces, w23);

    /*
    surface_t* s = make_sphere(
            3.0f, 1.0f, 5.0f,
            2.0f,
            &PURPLE, &PURPLE, &WHITE, 1000.0f) ;
    s->refr_index = 1.33f;
    s->atten = &DARK_GREY;
    lst_add(surfaces, s) ;
    */

    return surfaces ;

}

/** Create a list of lights.
 */
list356_t* get_lights() {
    list356_t* lights = make_list() ;

    light_t* l = malloc(sizeof(light_t)) ;
    l->position = malloc(sizeof(point3_t)) ;
    *(l->position) = (point3_t){50.0f, 20.0f, 0.0f} ;
    l->color = malloc(sizeof(color_t)) ;
    *(l->color) = (color_t){.5f, .5f, .5f} ;
    lst_add(lights, l) ;

    light_t* l2 = malloc(sizeof(light_t)) ;
    l2->position = malloc(sizeof(point3_t)) ;
    *(l2->position) = (point3_t){3.0f, 2.0f, 20.0f} ;
    l2->color = malloc(sizeof(color_t)) ;
    *(l2->color) = (color_t){.2f, .2f, .2f} ;
    lst_add(lights, l2) ;

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
