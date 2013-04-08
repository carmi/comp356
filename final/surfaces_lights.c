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

void rg(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 8.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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

    //// "Pieces"
    //for (int c=0; c<8; ++c) {
    //    for (int r=0; r<2; ++r) {
    //        surface_t* s = make_sphere(c+.5, r+.5, 1.375+.01, .375,
    //                &BLACK, &BLACK, &WHITE, 100.0f) ;
    //        lst_add(board_surfaces, s) ;
    //    }
    //    //for (int r=6; r<8; ++r) {
    //    //    lst_add(board_surfaces, make_sphere(c+.5, r+.5, 1.25, .25,
    //    //                &WHITE, &WHITE, &WHITE, 100.0f)) ;
    //    //}
    //}

    // Draw R manually
    // Define squares to place spheres on

    void add_sphere(c,r) {
        lst_add(board_surfaces, make_sphere(c+.5, r+.5, 1.75, .50,
                    &GREEN, &GREEN, &BLACK, 150.0f)) ;
    }
    //R
    //|
    add_sphere(0,2);
    add_sphere(0,3);
    add_sphere(0,4);
    add_sphere(0,5);
    add_sphere(0,6);

    // -
    add_sphere(1,6);

    add_sphere(2,6);
    add_sphere(2,5);
    add_sphere(2,3);
    add_sphere(3,2);

    add_sphere(1,4);

    //G
    add_sphere(7,6);
    add_sphere(6,6);
    add_sphere(5,5);
    add_sphere(5,4);
    add_sphere(5,3);
    add_sphere(5,3);
    add_sphere(6,2);
    add_sphere(7,2);
    add_sphere(7,3);
    add_sphere(7,4);

    lst_add(surfaces, make_bbt_node(board_surfaces)) ;
    lst_free(board_surfaces) ;
}
void chess(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 7.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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

void cube(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 4.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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
}

void sphere3(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 4.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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

    surface_t* sphere1 = make_sphere(6, 6, 1.75+.01, .75, &PURPLE, &PURPLE, &WHITE, 100.0f) ;
    //sphere1->refl_color = &RED;
    //sphere1->refl_color = &RED;
    lst_add(table_surfaces, sphere1) ;

    surface_t* sphere2 = make_sphere(5, 2, 1.75+.01, .75, &PURPLE, &PURPLE, &WHITE, 100.0f) ;
    sphere2->refr_index = 1.1f ;
    sphere2->atten = &WHITE ;
    lst_add(table_surfaces, sphere2) ;

    surface_t* sphere3 = make_sphere(3, 1, 1.75+.01, .75, &PURPLE, &PURPLE, &WHITE, 100.0f) ;
    lst_add(table_surfaces, sphere3) ;






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
}

void sphere(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 7.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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

void spheres(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 7.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

    // Spheres along the negative x, y, and z axes.
    list356_t* bbt_surfaces = make_list() ;
    surface_t* z_sphere ;
    surface_t* green_sphere;
    for (float x=0.0; x>=-400.0; x-=1.0) {
        lst_add(bbt_surfaces, (green_sphere = make_sphere(x, 0.0, 0.0f, 1.0f, &GREEN, 
                    &GREEN, &WHITE, 100.0f))) ;
        // green_sphere->refl_color = &LIGHT_GREY;
        lst_add(bbt_surfaces, make_sphere(0.0f, x, 0.0f, .25, &PURPLE, 
                    &PURPLE, &WHITE, 100.0f)) ;
        lst_add(bbt_surfaces, (z_sphere = make_sphere(0.0f, 0.0f, x, .25, &PURPLE, 
                    &PURPLE, &WHITE, 100.0f))) ;
        z_sphere->refl_color = &LIGHT_GREY ;
    }

    lst_add(bbt_surfaces, make_sphere(-9.0f, 1.0f, -9.0f, 2.0f, &WHITE, &WHITE, &WHITE, 90.0f));
    lst_add(bbt_surfaces, make_sphere(-9.0f, 1.0f, -8.0f, 2.0f, &WHITE, &WHITE, &WHITE, 90.0f));
    lst_add(bbt_surfaces, make_sphere(-9.0f, 1.0f, -6.0f, 2.0f, &WHITE, &WHITE, &WHITE, 90.0f));
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
    
    // An infinite plane in the y=-2 plane.
    /*
    surface_t* plane = make_plane(
                    (point3_t){-40, -2, 2},
                    (point3_t){2, -2, 2},
                    (point3_t){2, -2, -20},
                    &RED, &RED, &BLACK, 10.0f);
    plane->refl_color = &DARK_GREY;
    lst_add(surfaces, plane);
    */
    lst_add(surfaces, make_bbt_node(bbt_surfaces));

}

void walls(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {25.0f, 20.0f, -25.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, -1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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

    surface_t* s = make_sphere(
            3.0f, 1.0f, 5.0f,
            2.0f,
            &PURPLE, &PURPLE, &WHITE, 1000.0f) ;
    lst_add(surfaces, s) ;
}

/** Create a list of surfaces.
 */
void transcube(list356_t* surfaces, point3_t* eye, point3_t* look_at) {
    //update eye and look_at positions
    point3_t eye_position = {4.0f, -4.0f, 4.0f} ;
    point3_t look_at_point = {4.0f, 4.0f, 1.0f} ;
    *eye = eye_position;
    *look_at = look_at_point;

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
    MORE(surfaces, &eye_position, &look_at_point) ;

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
