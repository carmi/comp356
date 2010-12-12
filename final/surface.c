/** Surface functions.
 *
 *  @author N. Danner
 */

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "geom356.h"
#include "surface.h"

#define MALLOC1(t) (t *)(malloc(sizeof(t)))

#define min(a, b) (a < b ? a : b)

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

/** The type of a triangle surface.
 */
typedef struct _triangle_data_t triangle_data_t ;
/** The type of a plane surface.  A plane is specified by three points;
 *  the plane extends infinitely far in all directions.
 */
typedef struct _plane_data_t plane_data_t ;

/** The type of a sphere surface.
 */
typedef struct _sphere_data_t sphere_data_t ;
struct _sphere_data_t {
    /** The center of the sphere.
     */
    point3_t center ;
    /** The radius of the sphere.
     */
    float radius ;
} ;

struct _triangle_data_t {
    point3_t a, b, c ;
} ;

struct _plane_data_t {
    point3_t a, b, c ;
} ;


static void set_sfc_data(surface_t* surface, void* data,
        bool (*hit_fn)(void*, ray3_t*, float, float, hit_record_t*),
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) ;

static bool sfc_hit_sphere(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

static bool sfc_hit_tri(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

static bool sfc_hit_plane(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

surface_t* make_sphere(float x, float y, float z, float radius, 
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {
    sphere_data_t* data = MALLOC1(sphere_data_t) ;
    data->center = (point3_t){x, y, z} ;
    data->radius = radius ;

    surface_t* surface = MALLOC1(surface_t) ;
    set_sfc_data(surface, data, sfc_hit_sphere,
            diffuse_color, ambient_color, spec_color,
            phong_exp) ;

    return surface ;
}

surface_t* make_triangle(point3_t a, point3_t b, point3_t c,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {
    triangle_data_t* data = MALLOC1(triangle_data_t) ;
    data->a = a ;
    data->b = b ;
    data->c = c ;

    surface_t* surface = MALLOC1(surface_t) ;
    set_sfc_data(surface, data, sfc_hit_tri,
            diffuse_color, ambient_color, spec_color, phong_exp) ;

    return surface ;
}

surface_t* make_plane(point3_t a, point3_t b, point3_t c,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {
    plane_data_t* data = MALLOC1(plane_data_t) ;
    data->a = a ;
    data->b = b ;
    data->c = c ;

    surface_t* surface = MALLOC1(surface_t) ;
    set_sfc_data(surface, data, sfc_hit_plane,
            diffuse_color, ambient_color, spec_color, phong_exp) ;

    return surface ;
}


static void set_sfc_data(surface_t* surface, void* data,
        bool (*hit_fn)(void*, ray3_t*, float, float, hit_record_t*),
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) {
    surface->data = data ;
    surface->hit_fn = hit_fn ;
    surface->diffuse_color = diff;
    surface->ambient_color = amb;
    surface->spec_color = spec;
    surface->phong_exp = phong_exp ;
    surface->refl_color = NULL ;
}

static bool sfc_hit_sphere(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) {
    sphere_data_t* sdata = (sphere_data_t*)data ;
    point3_t ctr = sdata->center ;
    float radius = sdata->radius ;

    point3_t* e = &ray->base ;
    vector3_t* d = &ray->dir ;

    // First see if there is any chance we hit the sphere.  We do this
    // by checking whether any part of the sphere is inside the sphere
    // of radius t1 from the base of the ray.  With even just one sphere 
    // in the scene, I found that this check actually slowed down rendering,
    // probably because of all the square-root computations that turn
    // out to give no helpful information (i.e., this test says to
    // continue checking, but then the sphere isn't hit anyway).
    // if (t1 < (dist(e, &ctr) - radius)) return false ;

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
        point3_t* A, point3_t* B, point3_t* C,
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
        vector3_t b_minus_a, c_minus_a ;
        pv_subtract(B, A, &b_minus_a) ;
        pv_subtract(C, A, &c_minus_a) ;
        cross(&b_minus_a, &c_minus_a, &(hit->normal)) ;
        normalize(&(hit->normal)) ;
        multiply(&b_minus_a, beta, &b_minus_a) ;
        multiply(&c_minus_a, gamma, &c_minus_a) ;
        pv_add(A, &b_minus_a, &(hit->hit_pt)) ;
        pv_add(&(hit->hit_pt), &c_minus_a, &(hit->hit_pt)) ;
        return true ;
    }
    else return false ;
}

static bool sfc_hit_tri(void* data, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {
    triangle_data_t* tdata = (triangle_data_t*)data ;
    return sfc_hit_planar(true, &tdata->a, &tdata->b, &tdata->c,
            ray, t0, t1, hit) ;
}

static bool sfc_hit_plane(void* data, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {
    plane_data_t* pdata = (plane_data_t*)data ;
    return sfc_hit_planar(false, &pdata->a, &pdata->b, &pdata->c, ray, t0, t1, hit) ;
}

bool sfc_hit(surface_t* sfc, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {
    if ((sfc->hit_fn)(sfc->data, ray, t0, t1, hit)) {
        hit->sfc = sfc ;
        return true ;
    }
    else {
        return false ;
    }
}

