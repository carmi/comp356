/** Functions to load objects and lights.
 *
 */

#include <stdlib.h>

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

#include "debug.h"
#include "color.h"
#include "surface.h"

// Camera frame data.
point3_t eye_position = {4.0f, -4.0f, 7.0f} ;
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
color_t BLUE = {0.0, 0.0, 1.0} ;
color_t WHITE = {1.0f, 1.0f, 1.0f} ;
color_t LIGHT_GREY = {.6f, .6f, .6f} ;
color_t DARK_GREY = {.2f, .2f, .2f} ;
color_t BLACK = {0.0f, 0.0f, 0.0f} ;
color_t PURPLE = {.5f, .2f, .5f} ;
color_t GOLD = {255.0f/255, 215.0f/255, 0.0f} ;
color_t GREENISH = {1, .70f, 1} ;

void chess(list356_t* surfaces) {
    // Chess board.
    list356_t* board_surfaces = make_list() ;

    // All the vertices.
    point3_t vertices[85] ;
    for (int x=0; x<9; ++x) {
        for (int y=0; y<9; ++y) {
            vertices[9*y+x] = (point3_t){x, y, 1} ;
        }
    }
    vertices[81] = (point3_t){0, 0, -1} ;
    vertices[82] = (point3_t){8, 0, -1} ;
    vertices[83] = (point3_t){0, 8, -1} ;
    vertices[84] = (point3_t){8, 8, -1} ;

    int indices[138*3] ;
    int offset = 0 ;

    // Top of chess board.
    for (int x=0; x<8; ++x) {
        for (int y=0; y<8; ++y) {
            indices[offset++] = 9*y+x ;
            indices[offset++] = 9*y+x+1 ;
            indices[offset++] = 9*(y+1)+x+1 ;
            indices[offset++] = 9*y+x ;
            indices[offset++] = 9*(y+1)+x+1 ;
            indices[offset++] = 9*(y+1)+x ;
        }
    }
    int top_offset = offset ;

    // Sides and bottom.
    indices[offset++] = 81 ; indices[offset++] = 82; indices[offset++] = 8 ;
    indices[offset++] = 81 ; indices[offset++] = 8; indices[offset++] = 0 ;

    indices[offset++] = 8 ; indices[offset++] = 82 ; indices[offset++] = 84 ;
    indices[offset++] = 8 ; indices[offset++] = 84 ; indices[offset++] = 80 ;

    indices[offset++] = 72 ; indices[offset++] = 80 ; indices[offset++] = 84 ;
    indices[offset++] = 72 ; indices[offset++ ] = 84 ; indices[offset++] = 83 ;

    indices[offset++] = 0 ; indices[offset++] = 72 ; indices[offset++] = 83 ;
    indices[offset++] = 0 ; indices[offset++] = 83 ; indices[offset++] = 81 ;

    indices[offset++] = 81 ; indices[offset++] = 83 ; indices[offset++] =84 ;
    indices[offset++] = 81 ; indices[offset++] = 84 ; indices[offset++] = 82 ;
    debug("get_surfaces():  final offset = %d", offset) ;

    // Top as triangles.
    for (int i=0; i<top_offset/3; ++i) {
        int c = (i/2)%8 ;
        int r = (i/2)/8 ;
        int j = r+c ;
        color_t* color = j%2 == 0 ? &RED : &BLACK ;
        lst_add(board_surfaces, make_triangle(
                    vertices[indices[3*i]],
                    vertices[indices[3*i+1]],
                    vertices[indices[3*i+2]],
                    color, color, &WHITE, 10.0f)) ;
    }
    // Sides and bottom at triangles.
    for (int i=top_offset/3; i<offset/3; ++i) {
        lst_add(board_surfaces, make_triangle(
                    vertices[indices[3*i]],
                    vertices[indices[3*i+1]],
                    vertices[indices[3*i+2]],
                    &LIGHT_GREY, &LIGHT_GREY, &WHITE, 10.0f)) ;
    }

    // "Pieces"
    for (int c=0; c<8; ++c) {
        for (int r=0; r<2; ++r) {
            surface_t* s = make_sphere(c+.5, r+.5, 1.375+.01, .375,
                    &BLACK, &BLACK, &WHITE, 100.0f) ;
            lst_add(board_surfaces, s) ;
        }
        for (int r=6; r<8; ++r) {
            lst_add(board_surfaces, make_sphere(c+.5, r+.5, 1.25, .25,
                        &WHITE, &WHITE, &WHITE, 100.0f)) ;
        }
    }

    lst_add(surfaces, make_bbt_node(board_surfaces)) ;
    lst_free(board_surfaces) ;
}

void cube(list356_t* surfaces) {
    // A purple sphere.
    lst_add(surfaces, 
            make_sphere(6, 8, 0.75+.01, 1.75, 
                &PURPLE, &PURPLE, &WHITE, 100.0f)) ;

    // Transparent cube.
    point3_t cube_vertices[] = {
            {4, 2, 7}, {7, 2, 7}, {4, 3, 7}, {7, 3, 7},
            {4, 2, -.99}, {7, 2, -.99}, {4, 3, -.99}, {7, 3, -.99},
    } ;
    int indices[] = {
        0, 1, 3,    0, 3, 2,        // top
        0, 2, 6,    0, 6, 4,        // left
        4, 6, 7,    4, 7, 5,        // bottom
        1, 5, 7,    1, 7, 3,        // right
        2, 3, 7,    2, 7, 6,        // back
        0, 4, 5,    0, 5, 1         // front
    } ;
    int offset = 36 ;

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
}

void sphere(list356_t* surfaces) {
    // A purple sphere.
    lst_add(surfaces, 
            make_sphere(6, 9, 0.75+.01, 1.75, 
                &PURPLE, &PURPLE, &WHITE, 100.0f)) ;

    // A transparent sphere.
    surface_t* s = make_sphere(5, 2, 1.75+.01, 2.75,
            &BLACK, &BLACK, &WHITE, 1000.0f) ;
    s->refr_index = 1.05 ;
    s->atten = &GREENISH ;
    lst_add(surfaces, s) ;

}

/** Create a list of surfaces.
 */
list356_t* get_surfaces() {

    list356_t* surfaces = make_list() ;

    // Plane at z=-1.
    surface_t* plane = make_plane(
                (point3_t){0, 0, -1},
                (point3_t){1, 0, -1},
                (point3_t){1, 1, -1},
                &LIGHT_GREY, &LIGHT_GREY, &BLACK, 10.0f) ;
    plane->refl_color = &LIGHT_GREY ;
    lst_add(surfaces, plane) ;

    // MORE is intended to be preprocessor macro, so that it's meaning
    // can be changed at compile time.  E.g., one compile line might be
    //      $ CPPFLAGS=-DMORE=chess make final
    MORE(surfaces) ;

    return surfaces ;

}

/** Create a list of lights.
 */
list356_t* get_lights() {
    list356_t* lights = make_list() ;

    light_t* l = malloc(sizeof(light_t)) ;
    l->position = malloc(sizeof(point3_t)) ;
    *(l->position) = (point3_t){50.0f, 1.0f, 100.0f} ;
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
