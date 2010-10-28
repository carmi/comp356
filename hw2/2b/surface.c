/** Surface functions
 *  @file surface.c
 *  @brief - A ray tracing algorithm written in C.
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #2B
 *  Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
 *  ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
 *
 */

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "geom356.h"
#include "surface.h"

#define MALLOC1(t) (t *)(malloc(sizeof(t)))

/**
 * The type of a sphere. A sphere is specified by a center point (given by x-, y-, z-coordinates) and a radius.
 */
typedef struct _sphere_data_t {
    /** The center point given by x,y,z and radius of the sphere.
     */
    float   x;
    float   y;
    float   z;
    float radius;
} sphere_data_t;

/** The type of a triangle.  A triangle is specified by three points,
 *  which are the vertices of the triangle.  The surface normal
 *  points in the direction of (b-a) x (c-a).
 */
typedef struct _triangle_data_t {
    /** The three vertices of the triangle.
     */
    point3_t a, b, c ;
} triangle_data_t ;

typedef struct _plane_data_t {
    /** The three vertices of the plane.
     */
    point3_t a, b, c;
} plane_data_t;

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

/** Sphere-ray intersection function.
 *  
 *  @param data a <code>sphere_data_t</code> specifying the triangle.
 *  @param ray the ray for which to calculate intersection.
 *  @param t0 the minimum distance along the ray for the intersection.
 *  @param t1 the maximum distance along the ray for the intersection.
 *  @param hit the hit record to fill with data about the intersection.
 *      All attributes except the surface reference itself will be
 *      set if there is an intersection.
 *  @return <code>true</code> if <code>ray</code> intersects the
 *      sphere in the interval [t0, t1], <code>false</code> otherwise.
 */
static bool sfc_hit_sphere(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) ;

/** Plane-ray intersection function.
 *  
 *  @param data a <code>plane_data_t</code> specifying the triangle.
 *  @param ray the ray for which to calculate intersection.
 *  @param t0 the minimum distance along the ray for the intersection.
 *  @param t1 the maximum distance along the ray for the intersection.
 *  @param hit the hit record to fill with data about the intersection.
 *      All attributes except the surface reference itself will be
 *      set if there is an intersection.
 *  @return <code>true</code> if <code>ray</code> intersects the
 *      plane in the interval [t0, t1], <code>false</code> otherwise.
 */
static bool sfc_hit_plane(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit);
        
// SURFACE CREATION FUNCTIONS

surface_t* make_sphere(float x, float y, float z, float radius,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) {
    sphere_data_t* data = MALLOC1(sphere_data_t);
    data->x = x;
    data->y = y;
    data->z = z;
    data->radius = radius;

    surface_t* surface = MALLOC1(surface_t);
    set_sfc_data(surface, data, sfc_hit_sphere,
            diffuse_color, ambient_color, spec_color, phong_exp);

    return surface;
}

surface_t* make_triangle(point3_t a, point3_t b, point3_t c, color_t* diff,
        color_t* amb, color_t* spec, float phong_exp) {
    triangle_data_t* data = MALLOC1(triangle_data_t) ;
    data->a = a ;
    data->b = b ;
    data->c = c ;

    surface_t* surface = MALLOC1(surface_t) ;
    set_sfc_data(surface, data, sfc_hit_tri, diff, amb, spec, phong_exp) ;

    return surface ;
}

surface_t* make_plane(point3_t a, point3_t b, point3_t c, color_t* diff,
        color_t* amb, color_t* spec, float phong_exp) {
    plane_data_t* data = MALLOC1(plane_data_t);
    data->a = a;
    data->b = b;
    data->c = c;
    
    surface_t* surface = MALLOC1(surface_t);
    set_sfc_data(surface, data, sfc_hit_plane, diff, amb, spec, phong_exp);
    
    return surface;        
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

static bool sfc_hit_sphere(void* data, ray3_t* ray, float t0,
        float t1, hit_record_t* hit) {
    sphere_data_t* sdata = data;

    // Use the notation of Shirley & Marschner, Section 4.4.1.
    // t = (term1 +- discrim^.5)/denom
    // For general quadratic: term1 = -b, discrim = b^2 - 4ac, denom = 2a
    float R = sdata->radius;
    float R2 = R * R;

    // Create center point _c and pointer to it, c
    point3_t _c = (point3_t){sdata->x, sdata->y, sdata->z};
    point3_t* c = &_c;

    point3_t* e = &ray->base;
    vector3_t* d = &ray->dir;

    // We check if the sphere falls between t0 and t1.
    // First we need to find the points e + t0*d and e + t1*d.
    // vector3_t t0_times_d, t1_times_d;
    // multiply(d, t0, &t0_times_d);
    // multiply(d, t1, &t1_times_d);
    // point3_t t0_point, t1_point;
    // pv_add(e, &t0_times_d, &t0_point);
    // pv_add(e, &t1_times_d, &t1_point);
    // float eye_to_circle_edge_dist = dist(e, c) + R;
    // 
    // if ((eye_to_circle_edge_dist < dist(e, &t0_point)) ||
    //         (eye_to_circle_edge_dist > dist(e, &t1_point))) {
    //     return false;
    // }

    // We know c, d, e, and R, calculate t.

    // First calculate discriminant (term under square root)
    float d_dot_d = dot(d,d);
    float denom = d_dot_d;

    vector3_t e_minus_c;
    subtract((vector3_t*)e, (vector3_t*)c, &e_minus_c);

    // (d dot e_minus_c)^2
    // (d dot e_minus_c)
    float d_dot_ec = dot(d, &e_minus_c);
    float d_dot_ec2 = d_dot_ec * d_dot_ec;

    float discrim = d_dot_ec2 - d_dot_d * (dot(&e_minus_c, &e_minus_c) - R2);

    // If discrim is positve (or zero), there are solutions
    if (discrim >= 0) {
        vector3_t _neg_d = (vector3_t){-(d->x), -(d->y), -(d->z)};
        vector3_t* neg_d = &_neg_d;

        // (negative d) dot (e minus c)
        float term1 = dot(neg_d, &e_minus_c);
        float sqrt_discrim = (float)sqrt((double)discrim); 

        float t = (term1 + sqrt_discrim ) / denom;
        
        if (t <= t0 || t > t1) return false;
        
        // If discrim is zero, the ray grazes the sphere, touching it at
        // exactly one point.  Calculate second solution if discrim is not 0.
        if (!(discrim == 0)) {
            // Calculate t2, and if t2 < t, set t2 as t.
            float t2 = (term1 - sqrt_discrim ) / denom;
            if (t2 < t) {
                t = t2;
            }
        }
        hit->t = t;
        // fill in hit_pt and hit_normal vector
        // intersection point is e + td
        vector3_t td;
        multiply(d, t, &td);
        pv_add(e, &td, &(hit->hit_pt));

        // unit normal is (p-c)/R
        vector3_t p_minus_c;
        pv_subtract(&(hit->hit_pt), c, &p_minus_c);
        divide(&p_minus_c, R, &(hit->normal));
        return true;
    }
    // If discrim is negative, line and sphere do not intersect
    else return false;
}

static bool sfc_hit_plane(void* data, ray3_t* ray, float t0, float t1,
        hit_record_t* hit) {
    // Use notation the notation of Shirley & Marschner, Section 4.4.3.
    plane_data_t* pdata = data;
    point3_t* A = &pdata->a;
    point3_t* B = &pdata->b;
    point3_t* C = &pdata->c;
    
    // get normal.
    vector3_t b_minus_a, c_minus_a, n;
    pv_subtract(B, A, &b_minus_a);
    pv_subtract(C, A, &c_minus_a);
    cross(&b_minus_a, &c_minus_a, &n);
    normalize(&n);
    
    // solve for t. We can use any of the three points as our p1.
    vector3_t p1_minus_e;
    pv_subtract(A, &ray->base, &p1_minus_e);
    float numerator = dot(&p1_minus_e, &n);
    float denominator = dot(&ray->dir, &n);
    
    // if denominator is 0 then ray is parallel to plane and thus there's
    // no intersection.
    if (denominator == 0) return false;
    
    float t = numerator / denominator;
    
    if (t <= t0 || t > t1) return false;
    
    hit->t = t;
    
    // The intersection point is e + td. Fill in the hit record with it.
    vector3_t td;
    multiply(&ray->dir, t, &td);
    pv_add(&ray->base, &td, &(hit->hit_pt));
    
    hit->normal = n;
    
    return true;
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

