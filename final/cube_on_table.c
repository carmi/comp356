/** Functions to load objects and lights.
 *
 */

#include <stdlib.h>

#include "list356.h"

#include "debug.h"
#include "color.h"
#include "surface.h"

// Camera frame data.
point3_t eye_position = {4.0f, -4.0f, 4.0f} ;
point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
vector3_t up_dir = {0.0f, 0.0f, 1.0f} ;

// View-plane specification.
float view_plane_dist = 4.0f ;
float view_plane_width = 8.0f ;
float view_plane_height = 6.0f ;

// Ambient light.
color_t AMBIENT = {.1, .1, .1} ;

// Colors.
color_t RED = {1.0f, 0.0f, 0.0f} ;
color_t GREEN = {0.0f, 1.0f, 0.0f} ;
color_t WHITE = {1.0f, 1.0f, 1.0f} ;
color_t LIGHT_GREY = {.6f, .6f, .6f} ;
color_t BLACK = {0.0f, 0.0f, 0.0f} ;
color_t PURPLE = {.5f, .2f, .5f} ;
color_t GREENISH = {1, .70f, 1} ;

/** Create a list of surfaces.
 */
list356_t* get_surfaces() {

    list356_t* surfaces = make_list() ;

    // Table.
    point3_t vertices[8] = {
        {0, 0, 1}, {8, 0, 1}, {0, 8, 1}, {8, 8, 1},
        {0, 0, -1}, {8, 0, -1}, {0, 8, -1}, {8, 8, -1},
    } ;

    int indices[] = {
        0, 1, 3,    0, 3, 2,        // top
        0, 2, 6,    0, 6, 4,        // left
        4, 6, 7,    4, 7, 5,        // bottom
        1, 5, 7,    1, 7, 3,        // right
        2, 3, 7,    2, 7, 6,        // back
        0, 4, 5,    0, 5, 1         // front
    } ;
    int top_offset = 6 ;
    int offset = 36 ;

    list356_t* table_surfaces = make_list() ;
    for (int i=0; i<top_offset/3; ++i) {
        lst_add(table_surfaces, make_triangle(
                    vertices[indices[3*i]],
                    vertices[indices[3*i+1]],
                    vertices[indices[3*i+2]],
                    &RED, &RED, &WHITE, 10.0f)) ;
    }
    for (int i=top_offset/3; i<offset/3; ++i) {
        lst_add(table_surfaces, make_triangle(
                    vertices[indices[3*i]],
                    vertices[indices[3*i+1]],
                    vertices[indices[3*i+2]],
                    &GREEN, &GREEN, &WHITE, 10.0f)) ;
    }

    // Two purple spheres.
    lst_add(table_surfaces, 
            make_sphere(6, 6, 1.75+.01, .75, 
                &PURPLE, &PURPLE, &WHITE, 100.0f)) ;
    lst_add(table_surfaces, 
            make_sphere(5, 2, 1.75+.01, .75, 
                &PURPLE, &PURPLE, &WHITE, 100.0f)) ;

    // Transparent cube.
    point3_t cube_vertices[] = {
            {4, 0, 3}, {5, 0, 3}, {4, 1, 3}, {5, 1, 3},
            {4, 0, 1.01}, {5, 0, 1.01}, {4, 1, 1.01}, {5, 1, 1.01},
    } ;
    for (int i=0; i<offset/3; ++i) {
        surface_t* t = make_triangle(
                    cube_vertices[indices[3*i]],
                    cube_vertices[indices[3*i+1]],
                    cube_vertices[indices[3*i+2]],
                    &BLACK, &BLACK, &WHITE, 10.0f) ;
        t->refr_index = 1.1f ;
        t->atten = &GREENISH ;
        lst_add(surfaces, t) ;
    }

    list356_itr_t* itr = lst_iterator(table_surfaces) ;
    while (lst_has_next(itr)) lst_add(surfaces, lst_next(itr)) ;
    lst_free(table_surfaces) ;

    // Plane at z=-1.
    surface_t* plane = make_plane(
                (point3_t){0, 0, -1},
                (point3_t){1, 0, -1},
                (point3_t){1, 1, -1},
                &LIGHT_GREY, &LIGHT_GREY, &BLACK, 10.0f) ;
    plane->refl_color = &LIGHT_GREY ;
    lst_add(surfaces, plane) ;

    return surfaces ;

}

/** Create a list of lights.
 */
list356_t* get_lights() {
    list356_t* lights = make_list() ;

    light_t* l = malloc(sizeof(light_t)) ;
    l->position = malloc(sizeof(point3_t)) ;
    *(l->position) = (point3_t){50.0f, 2.0f, 100.0f} ;
    l->color = malloc(sizeof(color_t)) ;
    *(l->color) = (color_t){1.0f, 1.0f, 1.0f} ;
    lst_add(lights, l) ;

    light_t* l2 = malloc(sizeof(light_t)) ;
    l2->position = malloc(sizeof(point3_t)) ;
    *(l2->position) = (point3_t){4.0f, 12.0f, 20.0f} ;
    l2->color = malloc(sizeof(color_t)) ;
    *(l2->color) = (color_t){.2f, .2f, .2f} ;
    lst_add(lights, l2) ;

    return lights ;
}

void get_ambient_light(color_t* al) {
    *al = AMBIENT ;
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
