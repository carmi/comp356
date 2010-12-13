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

#include "surface.h"

#define MALLOC1(t) (t *)(malloc(sizeof(t)))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

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

/** Set standard surface data for a surface.  This function sets
 *  <code>refl_color</code> and <code>atten</code> to <code>NULL</code>
 *  and <code>refr_index</code> to <code>-1</code> (so by default,
 *  surfaces are opaque and have no reflectance).
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
        hit->sfc = sfc ;

        // Hit position.
        float num = min(-b - sqrt(discr), -b + sqrt(discr)) ;
        float t = num/d2 ;

        if (t < t0 || t > t1) return false ;

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

