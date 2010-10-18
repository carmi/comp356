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

/** The type of a triangle.  A triangle is specified by three points,
 *  which are the vertices of the triangle.  The surface normal
 *  points in the direction of (b-a) x (c-a).
 */
typedef struct _triangle_data_t {
    /** The three vertices of the triangle.
     */
    point3_t a, b, c ;
} triangle_data_t ;


/** A convenience function for setting the surface_t parameters.
 *  
 *  @param surface the surface.
 *  @param data the type-specific data (e.g., a triangle_data_t).
 *  @param hit_fn the ray-surface intersection function for
 *      <code>surface</code>.
 *  @param diff the diffuse color of the surface.
 *  @param amb the ambient color of the surface.
 *  @param spec the specular highlight color of the surface.
 *  @param phong_exp the phone exponent of the surface.
 */
static void set_sfc_data(surface_t* surface, void* data,
        bool (*hit_fn)(void*, ray3_t*, float, float, hit_record_t*),
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) ;

/** Triangle-ray intersection function.
 *  
 *  @param data a <code>triangle_data_t</code> specifying the triangle.
 *  @param ray the ray for which to calculate intersection.
 *  @param t0 the minimum distance along the ray for the intersection.
 *  @param t1 the maximum distance along the ray for the intersection.
 *  @param hit the hit record to fill with data about the intersection.
 *      All attributes except the surface reference itself will be
 *      set if there is an intersection.
 *  @return <code>true</code> if <code>ray</code> intersects the
 *      triangle in the interval [t0, t1], <code>false</code> otherwise.
 */
static bool sfc_hit_tri(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

// SURFACE CREATION FUNCTIONS

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

// SURFACE-RAY INTERSECTION FUNCTIONS.

static bool sfc_hit_tri(void* data,
        ray3_t* ray, float t0, float t1, hit_record_t* hit) {

    triangle_data_t* tdata = data ;
    point3_t* A = &tdata->a ;
    point3_t* B = &tdata->b ;
    point3_t* C = &tdata->c ;

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
    if (beta < 0 || beta > 1) return false ;

    float gamma = (i*ak_jb + h*jc_al + g*bl_kc)/M ;

    if (0 <= gamma && beta+gamma <= 1) {
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

