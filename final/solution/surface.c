/** Surface functions.
 *
 *  @author N. Danner
 */

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#elif defined __LINUX__ || defined __CYGWIN__
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "geom356.h"
#include "list356.h"

#include "surface.h"

#define MALLOC1(t) (t *)(malloc(sizeof(t)))

#define min(a, b) (a < b ? a : b)

// Ratio of the bounding-box extent by which to jitter surfaces when building
// a BBT.
#define JITTER_RATIO .01

// A marginally-clever debugging function that expends to a no-op
// when NDEBUG is defined and otherwise prints its string formatting
// arguments to stderr.
#ifdef NDEBUG
#define debug(fmt, ...) 
#define debug_c(expr, fmt, ...)
#else
static void debug(const char* fmt, ...) {
    va_list argptr ;
    va_start(argptr, fmt) ;

    char* fmt_newline ;
    asprintf(&fmt_newline, "%s\n", fmt) ;
    vfprintf(stderr, fmt_newline, argptr) ;
    free(fmt_newline) ;
    fflush(stderr) ;
}
static void debug_c(bool expr, const char* fmt, ...) {
    if (expr) {
        va_list argptr ;
        va_start(argptr, fmt) ;

        char* fmt_newline ;
        asprintf(&fmt_newline, "%s\n", fmt) ;
        vfprintf(stderr, fmt_newline, argptr) ;
        free(fmt_newline) ;
        fflush(stderr) ;
    }
}
#endif

/** The type of a sphere surface.
 */
typedef struct _sphere_data_t {
    /** The center of the sphere.
     */
    point3_t center ;
    /** The radius of the sphere.
     */
    float radius ;
} sphere_data_t ;

/** The type of a triangle surface.
 */
typedef struct _triangle_data_t {
    point3_t a, b, c ;
    vector3_t normal ;
} triangle_data_t ;

/** The type of a plane surface.  A plane is specified by three points;
 *  the plane extends infinitely far in all directions.
 */
typedef struct _plane_data_t {
    point3_t a, b, c ;
    vector3_t normal ;
} plane_data_t ;

/* The type of a triangulated surface.  The surface is specified by an
 * indexed triangle mesh.  I am leaving this out for now, because
 * this sort of surface complicates bounding-box tree construction;
 * the latter is useful only when we have direct access to the
 * triangles, but to get that access, we have to implement a method
 * for every surface, which is just too much of a pain for the benefit
 * right now.
typedef struct _poly_surface_data_t {
    list356_t* triangles ;
} poly_surface_data_t ;
*/

/** The type of a bounding-box tree node.  The bounding box of this
 *  type of surface bounds all the surfaces below it.
 */
typedef struct _bbt_node_data_t {
    surface_t* left ;
    surface_t* right ;
} bbt_node_data_t ;

/** Set standard surface data for a surface.
 *  
 *  @param surface the surface for which to set the data.
 *  @param data the type-specific data for the surface.
 *  @param hit_fn the hit function for the surface.
 *  @param diff the diffuse color for the surface.
 *  @param spec the specular highlight color for the surface.
 *  @param phong_exp the Blinn-Phong exponent for the surface.
 */
static void set_sfc_data(surface_t* surface, void* data,
        bool (*hit_fn)(surface_t*, ray3_t*, float, float, hit_record_t*),
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) ;

/** Sphere-ray intersection function.
 *  
 *  @param sfc the sphere surface.
 *  @param ray the ray.
 *  @param t0 the left endpoint of the ray.
 *  @param t1 the right endpoint of the ray.
 *  @param hit the hit record to fill in if <code>ray</code> hits
 *      <code>sfc</code>.
 *
 *  @return <code>true</code> if <code>ray</code> intersects 
 *      <code>sfc</code> in the interval [<code>t0</code>,<code>t1</code>],
 *      <code>false</code> otherwise.  If there is an intersection,
 *      <code>hit</code> will be populated with data describing the
 *      intersection point; otherwise <code>hit</code> will be unmodified.
 */
static bool sfc_hit_sphere(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

/** Triangle-ray intersection function.
 *  
 *  @param sfc the triangle surface.
 *  @param ray the ray.
 *  @param t0 the left endpoint of the ray.
 *  @param t1 the right endpoint of the ray.
 *  @param hit the hit record to fill in if <code>ray</code> hits
 *      <code>sfc</code>.
 *
 *  @return <code>true</code> if <code>ray</code> intersects 
 *      <code>sfc</code> in the interval [<code>t0</code>,<code>t1</code>],
 *      <code>false</code> otherwise.  If there is an intersection,
 *      <code>hit</code> will be populated with data describing the
 *      intersection point; otherwise <code>hit</code> will be unmodified.
 */
static bool sfc_hit_tri(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

/** Plane-ray intersection function.
 *  
 *  @param sfc the plane surface.
 *  @param ray the ray.
 *  @param t0 the left endpoint of the ray.
 *  @param t1 the right endpoint of the ray.
 *  @param hit the hit record to fill in if <code>ray</code> hits
 *      <code>sfc</code>.
 *
 *  @return <code>true</code> if <code>ray</code> intersects 
 *      <code>sfc</code> in the interval [<code>t0</code>,<code>t1</code>],
 *      <code>false</code> otherwise.  If there is an intersection,
 *      <code>hit</code> will be populated with data describing the
 *      intersection point; otherwise <code>hit</code> will be unmodified.
 */
static bool sfc_hit_plane(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

/*
static bool sfc_hit_poly(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;
        */

/** BBT node-ray intersection function.
 *  
 *  @param sfc the BBT node surface.
 *  @param ray the ray.
 *  @param t0 the left endpoint of the ray.
 *  @param t1 the right endpoint of the ray.
 *  @param hit the hit record to fill in if <code>ray</code> hits
 *      <code>sfc</code>.
 *
 *  @return <code>true</code> if <code>ray</code> intersects 
 *      <code>sfc</code> in the interval [<code>t0</code>,<code>t1</code>],
 *      <code>false</code> otherwise.  If there is an intersection,
 *      <code>hit</code> will be populated with data describing the
 *      intersection point; otherwise <code>hit</code> will be unmodified.
 */
static bool sfc_hit_bbt_node(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

/** Bounding-box-ray intersection function.
 *  
 *  @param bbox a world-frame axis-aligned bounding box.
 *  @param ray the ray.
 *  @param t0 the left endpoint of the ray.
 *  @param t1 the right endpoint of the ray.
 *
 *  @return <code>true</code> if <code>ray</code> intersects 
 *      <code>sfc</code> in the interval [<code>t0</code>,<code>t1</code>],
 *      <code>false</code> otherwise.
 */
static bool hit_bbox(bbox_t* bbox, ray3_t* ray, float t0, float t1) ;

/** Combine two bounding boxes.
 *  
 *  @param b1 an axis-aligned bounding box.
 *  @param b2 an axis-aligned bounding box.
 *  @param r the result.  <code>r</code> will be set to be the smallest
 *      axis-aligned bounding box that contains <code>b1</code> and
 *      <code>b2</code>.
 */
static void combine_bbox(bbox_t* b1, bbox_t* b2, bbox_t* r) ;

/** Merge two bounding boxes.  This is equivalent to
 *  <pre>
 *  combine_bbox(new, current, current)
 *  </pre>
 *  
 *  @param new an axis-aligned bounding box.
 *  @param current an axis-aligned bounding box.
 */
static void merge_bbox(bbox_t* new, bbox_t* current) ;

/** Get the lower value of an axis-aligned boudning box along a given axis.
 *
 *  @param b an axis-aligned bounding box.
 *  @param axis an axis.  0 means x-axis, 1 means y-axis, 2 means z-axis.
 *
 *  @return the lower value of <code>b</code> along <code>axis</code>
 *      (i.e., <code>left</code>, <code>bottom</code>, or <code>near</code>).
 */
static float bbox_lower(bbox_t* b, int axis) ;

/** Get the upper value of an axis-aligned boudning box along a given axis.
 *
 *  @param b an axis-aligned bounding box.
 *  @param axis an axis.  0 means x-axis, 1 means y-axis, 2 means z-axis.
 *
 *  @return the upper value of <code>b</code> along <code>axis</code>
 *      (i.e., <code>right</code>, <code>top</code>, or <code>far</code>).
 */
static float bbox_upper(bbox_t* b, int axis) ;

/** Compute the midpoint of an axis-aligned bounding box along
 *  a given axis.
 *
 *  @param b an axis-aligned bounding box.
 *  @param axis an axis.  0 means x-axis, 1 means y-axis, 2 means z-axis.
 *
 *  @return the midpoing of <code>b</code> along <code>axis</code>.
 */
static float bbox_midpoint(bbox_t* b, int axis) ;

//
// UTILITY FUNCTIONS.
//

/** Compute the minimum of four numbers.
 *  
 *  @param v a number.
 *  @param a a number.
 *  @param b a number.
 *  @param c a number.
 *  
 *  @return min(v, a, b, c).
 */
static float min4(float v, float a, float b, float c) {
    return min(v, min(a, min(b, c))) ;
}

#define max(a, b) ((a) > (b) ? (a) : (b))

/** Compute the maximum of four numbers.
 *  
 *  @param v a number.
 *  @param a a number.
 *  @param b a number.
 *  @param c a number.
 *  
 *  @return max(v, a, b, c).
 */
static float max4(float v, float a, float b, float c) {
    return max(v, max(a, max(b, c))) ;
}

surface_t* make_sphere(float x, float y, float z, float radius, 
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {

    // Sphere data.
    sphere_data_t* data = MALLOC1(sphere_data_t) ;
    data->center = (point3_t){x, y, z} ;
    data->radius = radius ;

    surface_t* surface = MALLOC1(surface_t) ;

    // Bounding box.
    surface->bbox = MALLOC1(bbox_t) ;
    surface->bbox->left = x-radius ;
    surface->bbox->right = x+radius ;
    surface->bbox->bottom = y-radius ;
    surface->bbox->top = y+radius ;
    surface->bbox->near = z-radius ;
    surface->bbox->far = z+radius ;

    set_sfc_data(surface, data, sfc_hit_sphere,
            diffuse_color, ambient_color, spec_color,
            phong_exp) ;

    return surface ;
}

surface_t* make_triangle(point3_t a, point3_t b, point3_t c,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {

    // Triangle data.  Pre-compute the normal, because it is the same
    // for all points on the triangle.
    triangle_data_t* data = MALLOC1(triangle_data_t) ;
    data->a = a ;
    data->b = b ;
    data->c = c ;
    vector3_t BA, CA;
    pv_subtract(&b, &a, &BA) ;
    pv_subtract(&c, &a, &CA) ;
    cross(&BA, &CA, &(data->normal)) ;
    normalize(&(data->normal)) ;

    surface_t* surface = MALLOC1(surface_t) ;

    // Axis-aligned bounding box; this need not be two-dimensional
    // (e.g., if the triangle does not lie in one of the world-frame
    // axis planes).
    surface->bbox = MALLOC1(bbox_t) ;
    surface->bbox->left = min4(FLT_MAX, a.x, b.x, c.x) ;
    surface->bbox->right = max4(FLT_MIN, a.x, b.x, c.x) ;
    surface->bbox->bottom = min4(FLT_MAX, a.y, b.y, c.y) ;
    surface->bbox->top = max4(FLT_MIN, a.y, b.y, c.y) ;
    surface->bbox->near = min4(FLT_MAX, a.z, b.z, c.z) ;
    surface->bbox->far = max4(FLT_MIN, a.z, b.z, c.z) ;

    set_sfc_data(surface, data, sfc_hit_tri,
            diffuse_color, ambient_color, spec_color, phong_exp) ;

    return surface ;
}

surface_t* make_plane(point3_t a, point3_t b, point3_t c,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {

    // Plane data.  Like a triangle, pre-compute the surface normal.
    plane_data_t* data = MALLOC1(plane_data_t) ;
    data->a = a ;
    data->b = b ;
    data->c = c ;
    vector3_t BA, CA;
    pv_subtract(&b, &a, &BA) ;
    pv_subtract(&c, &a, &CA) ;
    cross(&BA, &CA, &(data->normal)) ;
    normalize(&(data->normal)) ;

    // No bounding box, because that just doesn't make sense for a
    // surface that extends infinitely far in all directions!
    surface_t* surface = MALLOC1(surface_t) ;
    surface->bbox = NULL ;
    set_sfc_data(surface, data, sfc_hit_plane,
            diffuse_color, ambient_color, spec_color, phong_exp) ;

    return surface ;
}

/* As mentioned above, we aren't doing triangulated surfaces right now.

// Transformation functions for poly_surface.
// Convert a matrix in column-major order to row-major order.
static void column_to_row_major(GLfloat* mc, GLfloat* mr) {
    for (int i=0; i<4; ++i) {
        for (int j=0; j<4; ++j) {
            mr[4*j+i] = mc[4*i+j] ;
        }
    }
}

// Dot product of two 4-D vectors.
static float dot4(GLfloat* v1, float*v2) {
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2] + v1[3]*v2[3] ;
}

// Transform a point by a matrix given in row-major order.
static void mult_matrix_point(GLfloat* m, point3_t* p, point3_t* result) {
    // p in homogeneous coordinates.
    float v[4] = {p->x, p->y, p->z, 1.0} ;

    result->x = dot4(v, m) ;
    result->y = dot4(v, m+4) ;
    result->z = dot4(v, m+8) ;
}

surface_t* make_poly_surface(point3_t* orig_vertices, int num_vertices,
        int* indices, int num_indices, GLfloat* xfrm,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {

    poly_surface_data_t* data = MALLOC1(poly_surface_data_t) ;
    data->triangles = make_list() ;

    // Bounding box data.
    float left = FLT_MAX, bottom = FLT_MAX, near = FLT_MAX ;
    float right = FLT_MIN, top = FLT_MIN, far = FLT_MIN ;

    // Transformed vertices.
    point3_t* vertices ;
    if (xfrm == NULL) vertices = orig_vertices ;
    else {
        vertices = malloc(num_vertices*sizeof(point3_t)) ;
        GLfloat xfrm_row_major[16] ;
        column_to_row_major(xfrm, xfrm_row_major) ;
        for (int i=0; i<num_vertices; ++i) {
            mult_matrix_point(xfrm_row_major, orig_vertices+i, vertices+i) ;
        }
    }

 
    int num_triangles = num_indices/3 ;
    for (int i=0; i<num_triangles; ++i) {
        point3_t a = vertices[indices[3*i]] ;
        point3_t b = vertices[indices[3*i+1]] ;
        point3_t c = vertices[indices[3*i+2]] ;

        lst_add(data->triangles, make_triangle(a, b, c,
                    diffuse_color, ambient_color, spec_color, phong_exp)) ;

        left = min4(left, a.x, b.x, c.x) ;
        right = max4(right, a.x, b.x, c.x) ;
        bottom = min4(bottom, a.y, b.y, c.y) ;
        top = max4(top, a.y, b.y, c.y) ;
        near = min4(near, a.z, b.z, c.z) ;
        far = max4(far, a.z, b.z, c.z) ;
    }

    surface_t* surface = MALLOC1(surface_t) ;
    surface->bbox = MALLOC1(bbox_t) ;
    surface->bbox->left = left ;
    surface->bbox->right = right ;
    surface->bbox->bottom = bottom ;
    surface->bbox->top = top ;
    surface->bbox->near = near ;
    surface->bbox->far = far ;

    set_sfc_data(surface, data, sfc_hit_poly,
            diffuse_color, ambient_color, spec_color, phong_exp) ;

    if (xfrm != NULL) free(vertices) ;

    return surface ;
}
*/

static int max_depth = 0 ;
/** Recursively construct a BBT node.
 *  
 *  @param surfaces a list of surfaces, each of which has a non-NULL
 *      bounding box.
 *  @param axis the along which to split the surfaces.  0 means
 *      x-axis, 1 means y-axis, 2 means z-axis.
 *
 *  @return a BBT node containing every surface in <code>surfaces</code>
 *      as a descendent.
 */
static surface_t* make_bbt_node_rec(list356_t* surfaces, int axis, int depth) {
    max_depth = max_depth > depth ? max_depth : depth ;

    // Slightly ugly to split up the base cases, but if there is only
    // one surface, it *is* the node.
    if (lst_size(surfaces) == 1) return lst_get(surfaces, 0) ;

    // Otherwise we make a real BBT node.
    surface_t* node = MALLOC1(surface_t) ;
    node->data = MALLOC1(bbt_node_data_t) ;
    bbt_node_data_t* node_data = node->data ;
    
    // Next base case:  two surfaces.
    if (lst_size(surfaces) == 2) {
        node_data->left = lst_get(surfaces, 0) ;
        node_data->right = lst_get(surfaces, 1) ;
        node->bbox = MALLOC1(bbox_t) ;
        combine_bbox(node_data->left->bbox, node_data->right->bbox, 
                node->bbox) ;
    }
    else {
        // Determine the box that bounds all the surfaces.
        node->bbox = MALLOC1(bbox_t) ;
        node->bbox->left = node->bbox->bottom = node->bbox->near = FLT_MAX ;
        node->bbox->right = node->bbox->top = node->bbox->far = FLT_MIN ;
 
        list356_itr_t* itr = lst_iterator(surfaces) ;
        while (lst_has_next(itr)) {
            merge_bbox(((surface_t*)lst_next(itr))->bbox, node->bbox) ;
        }
        lst_iterator_free(itr) ;

        // The midpoint of the bounding box along axis.
        float m = bbox_midpoint(node->bbox, axis) ;
        float jitter = 
            (bbox_upper(node->bbox, axis) - bbox_lower(node->bbox, axis))*
                                                                JITTER_RATIO ;

        // Split surfaces into two lists.  For each surface s, compute
        // the "jittered" midpoint of its bounding box along axis
        // and put into s1 or s2 according to whether that jittered
        // midpoint is <=m or >m.  We jitter the midpoint so that we
        // avoid (with high probability) an infinite recursion caused
        // by peculiar lists of surfaces in which there are some surfaces
        // whose extent is the entire bounding box.
        //
        // An even better approach that some of you took is the following.
        // First test whether the surface bbox midpoint is exactly m.
        // If so, flip a fair coin (rand()%2 == 0) and put the surface
        // in one list or the other on that basis.  If the surface bbox
        // midpoint is not exactly m, put into s1 or s1 as usual.
        list356_t* s1 = make_list() ;
        list356_t* s2 = make_list() ;
        itr = lst_iterator(surfaces) ;
        while (lst_has_next(itr)) {
            surface_t* s = lst_next(itr) ;
            float mid = bbox_midpoint(s->bbox, axis) ;
            mid += (2*(float)rand()/RAND_MAX - 1.0f)*jitter ;
            if (mid <= m) lst_add(s1, s) ;
            else lst_add(s2, s) ;
        }
        lst_iterator_free(itr) ;
        debug("make_bbt_node_rec(): partition size = %d, %d",
                lst_size(s1), lst_size(s2)) ;

        // It is still possible that all the surfaces end up in one
        // list, so we have to take that possibility into account.
        node_data->left = lst_size(s1) > 0 ? 
            make_bbt_node_rec(s1, (axis+1)%3, depth+1) : NULL ;
        node_data->right = lst_size(s2) > 0 ?
            make_bbt_node_rec(s2, (axis+1)%3, depth+1) : NULL ;

        lst_free(s1) ;
        lst_free(s2) ;
        
    }

    node->hit_fn = sfc_hit_bbt_node ;
    node->refr_index = -1.0f ;
    node->atten = NULL ;
    return node ;
}

surface_t* make_bbt_node(list356_t* surfaces) {
    surface_t* n = make_bbt_node_rec(surfaces, 0, 0) ;
    debug("make_bbt_node(): depth = %d", max_depth) ;
    return n ;
}

static void set_sfc_data(surface_t* surface, void* data,
        bool (*hit_fn)(surface_t*, ray3_t*, float, float, hit_record_t*),
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) {
    surface->data = data ;
    surface->hit_fn = hit_fn ;
    surface->diffuse_color = diff;
    surface->ambient_color = amb;
    surface->spec_color = spec;
    surface->phong_exp = phong_exp ;
    surface->refl_color = NULL ;
    surface->refr_index = -1.0f ;
    surface->atten = NULL ;
}

//
// SURFACE-RAY INTERSECTION FUNCTIONS
//

static bool sfc_hit_sphere(surface_t* sfc, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) {

    // It is faster to check the discriminant than the bounding box,
    // so we don't bother with the latter.
    // if (!hit_bbox(sfc->bbox, ray, t0, t1)) return false ;

    sphere_data_t* sdata = (sphere_data_t*)(sfc->data) ;
    point3_t ctr = sdata->center ;
    float radius = sdata->radius ;

    point3_t* e = &ray->base ;
    vector3_t* d = &ray->dir ;

    vector3_t e_minus_ctr ;
    pv_subtract(e, &ctr, &e_minus_ctr) ;
    float e_minus_ctr2 = dot(&e_minus_ctr, &e_minus_ctr) ;
    float d2 = dot(d, d) ;

    // Compute the discriminant first.
    float b = dot(d, &e_minus_ctr) ;
    float discr = 
        b*b - d2*(e_minus_ctr2 - radius*radius) ;

    // Compute hit position if discr. is >= 0, and also compute
    // the surface normal.
    if (discr < 0) return false ;
    else {

        // Hit position.  We have to be careful now that we allow
        // for refraction---the ray may start *inside* the sphere,
        // in which case just taking the minimum does not do the job!
        /*
        float num = min(-b - sqrt(discr), -b + sqrt(discr)) ;
        float t = num/d2 ;
        if (t < t0 || t > t1) return false ;
        */

        // Intersection points with the sphere.
        float s0 = (-b-sqrt(discr))/d2 ;
        float s1 = (-b+sqrt(discr))/d2 ;

        float t ;
        if (s0 > 0) {   // Ray starts before first int. point; want s0.
            t = s0 ;
        }
        else {          // Ray starts after first int. point; want s1.
            t = s1 ;
        }
        if (t < t0 || t > t1) return false ;

        hit->sfc = sfc ;
        hit->t = t ;

        vector3_t ray_vec = *d ;
        multiply(&ray_vec, hit->t, &ray_vec) ;
        pv_add(e, &ray_vec, &(hit->hit_pt)) ;

        // Surface normal.
        pv_subtract(&(hit->hit_pt), &ctr, &(hit->normal)) ;
        normalize(&(hit->normal)) ;
        return true ;
    }
}

static bool sfc_hit_planar(bool is_triangle, 
        point3_t* A, point3_t* B, point3_t* C, vector3_t* normal,
        ray3_t* ray, float t0, float t1, hit_record_t* hit) {

    // Use the notation of Shirley & Marschner, Section 4.4.2.
    float a = A->x - B->x ;
    float b = A->y - B->y ;
    float c = A->z - B->z ;
    float d = A->x - C->x ;
    float e = A->y - C->y ;
    float f = A->z - C->z ;
    float g = ray->dir.x ;
    float h = ray->dir.y ;
    float i = ray->dir.z ;
    float j = A->x - ray->base.x ;
    float k = A->y - ray->base.y ;
    float l = A->z - ray->base.z ;

    float ei = e*i, hf = h*f, gf = g*f, di = d*i, dh = d*h, eg = e*g ;
    float ak = a*k, jb = j*b, jc = j*c, al = a*l, bl = b*l, kc = k*c ;

    float ei_hf = ei-hf, gf_di = gf-di, dh_eg = dh-eg ;
    float ak_jb = ak-jb, jc_al = jc-al, bl_kc = bl-kc ;

    float M = a*ei_hf + b*gf_di + c*dh_eg ;

    float t = -(f*ak_jb + e*jc_al + d*bl_kc)/M ;

    if (t <= t0 || t > t1) return false ;

    float beta = (j*ei_hf + k*gf_di + l*dh_eg)/M ;
    if (is_triangle && (beta < 0 || beta > 1)) return false ;

    float gamma = (i*ak_jb + h*jc_al + g*bl_kc)/M ;

    if (!is_triangle || (0 <= gamma && beta+gamma <= 1)) {
        hit->t = t ;
        hit->normal = *normal ;
        vector3_t b_minus_a, c_minus_a ;
        pv_subtract(B, A, &b_minus_a) ;
        pv_subtract(C, A, &c_minus_a) ;
        multiply(&b_minus_a, beta, &b_minus_a) ;
        multiply(&c_minus_a, gamma, &c_minus_a) ;
        pv_add(A, &b_minus_a, &(hit->hit_pt)) ;
        pv_add(&(hit->hit_pt), &c_minus_a, &(hit->hit_pt)) ;
        return true ;
    }
    else return false ;
}

static bool sfc_hit_tri(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {
    if (hit_bbox(sfc->bbox, ray, t0, t1)) {
        triangle_data_t* tdata = (triangle_data_t*)(sfc->data) ;
        if (sfc_hit_planar(true, &tdata->a, &tdata->b, &tdata->c, 
                    &tdata->normal,
                ray, t0, t1, hit)) {
            hit->sfc = sfc ;
            return true ;
        }
    }

    return false ;
}

static bool sfc_hit_plane(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {

    // We don't check the bounding box, because planar surfaces do not
    // have bounding boxes!
    plane_data_t* pdata = (plane_data_t*)(sfc->data) ;
    if (sfc_hit_planar(false, &pdata->a, &pdata->b, &pdata->c, 
            &pdata->normal, ray, t0, t1, hit)) {
        hit->sfc = sfc ;
        return true ;
    }
    else return false ;
}

/* As mentioned above, we are not doing triangulated surfaces right now.
static bool sfc_hit_poly(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {

    bool hit_something = false ;
    // if (hit_bbox(sfc->bbox, ray, t0, t1)) {
        poly_surface_data_t* D = sfc->data ;
        list356_itr_t* itr = lst_iterator(D->triangles) ;
        float t = t1 ;
        while (lst_has_next(itr)) {
            surface_t* tri = lst_next(itr) ;
            if (sfc_hit(tri, ray, t0, t, hit)) {
                hit_something = true ;
                t = hit->t ;
            }
        }
    // }
    return hit_something ;
}
*/

static bool sfc_hit_bbt_node(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {

    if (hit_bbox(sfc->bbox, ray, t0, t1)) {
        hit_record_t rec_left, rec_right ;
        bool hit_left, hit_right ;

        bbt_node_data_t* data = sfc->data ;
        hit_left = (data->left != NULL) && 
            sfc_hit(data->left, ray, t0, t1, &rec_left) ;
        hit_right = (data->right != NULL) &&
            sfc_hit(data->right, ray, t0, t1, &rec_right) ;

        if (hit_left && hit_right) {
            memcpy(hit, (rec_left.t < rec_right.t ? &rec_left : &rec_right),
                    sizeof(hit_record_t)) ;
        }
        else if (hit_left) memcpy(hit, &rec_left, sizeof(hit_record_t)) ;
        else if (hit_right) memcpy(hit, &rec_right, sizeof(hit_record_t)) ;

        if (hit_left || hit_right) return true ;
    }
    return false ;

}

bool sfc_hit(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {

    return sfc->hit_fn(sfc, ray, t0, t1, hit) ;

}

//
// BOUNDING BOX FUNCIONS.
//

static bool hit_bbox(bbox_t* bbox, ray3_t* ray, float t0, float t1) {

    // Intersection times, following lecture notes.
    float txmin, txmax, tymin, tymax, tzmin, tzmax ;

    // Convenience:  ray = e+td.
    point3_t e = ray->base ;
    vector3_t d = ray->dir ;

    // Intersection times with x-axis cutting planes.
    float ax = 1.0/d.x ;
    txmin = ((ax >= 0 ? bbox->left : bbox->right) - e.x)*ax ;
    txmax = ((ax >= 0 ? bbox->right : bbox->left) - e.x)*ax ;

    // Intersection times with y-axis cutting planes.
    float ay = 1.0/d.y ;
    tymin = ((ay >= 0 ? bbox->bottom : bbox->top) - e.y)*ay ;
    tymax = ((ay >= 0 ? bbox->top : bbox->bottom) - e.y)*ay ;

    // Intersection times with x-axis cutting planes.
    float az = 1.0/d.z ;
    tzmin = ((az >= 0 ? bbox->near : bbox->far) - e.z)*az ;
    tzmax = ((az >= 0 ? bbox->far : bbox->near) - e.z)*az ;

    return (txmin <= tymax) && (txmax >= tymin) &&
        (txmin <= tzmax) && (txmax >= tzmin) &&
        (tymin <= tzmax) && (tymax >= tzmin) ;

}

static void combine_bbox(bbox_t* b1, bbox_t* b2, bbox_t* r) {
    r->left = min(b1->left, b2->left) ;
    r->right = max(b1->right, b2->right) ;
    r->bottom = min(b1->bottom, b2->bottom) ;
    r->top = max(b1->top, b2->top) ;
    r->near = min(b1->near, b2->near) ;
    r->far = max(b1->far, b2->far) ;
}

static void merge_bbox(bbox_t* new, bbox_t* current) {
    current->left = min(current->left, new->left) ;
    current->right = max(current->right, new->right) ;
    current->bottom = min(current->bottom, new->bottom) ;
    current->top = max(current->top, new->top) ;
    current->near = min(current->near, new->near) ;
    current->far = max(current->far, new->far) ;
}

static float bbox_lower(bbox_t* b, int axis) {
    float extents[3][2] = {{b->left, b->right}, {b->bottom, b->top},
        {b->near, b->far}} ;
    return extents[axis][0] ;
}

static float bbox_upper(bbox_t* b, int axis) {
    float extents[3][2] = {{b->left, b->right}, {b->bottom, b->top},
        {b->near, b->far}} ;
    return extents[axis][1] ;
}

static float bbox_midpoint(bbox_t* b, int axis) {
    return (bbox_upper(b, axis) + bbox_lower(b, axis))/2.0f ;
}
