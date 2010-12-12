/** @file surface.h Surface types and functions.
 *
 *  @author N. Danner
 */

#ifndef SURFACE_H
#define SURFACE_H

#include <stdbool.h>

#include "color.h"
#include "geom356.h"

/** The type of a hit record.  The structure is exposed below.
 */
typedef struct _hit_record_t hit_record_t ;

/** The type of a surface.  This structure is exposed below.
 */
typedef struct _surface_t surface_t ;

/** The type of a point light source.  The structure is exposed below.
 */
typedef struct _light_t light_t ;

/** The surface structure.  We expose its definition so as to make direct
 *  access to the components simpler.
 */
struct _surface_t {
    /** Data that is specific to the type of surface.
     */
    void*           data ;
    /** The ray-surface intersection function for this surface.
     *  
     *  @param data This will be a <code>sphere_data_t</code>,
     *      <code>triangle_data_t</code>, etc., depending on what
     *      type of surface this is.
     *  @param ray the ray for which to compute the intersection.
     *  @param t0 the minimum intersection time that is valid.
     *  @param t1 the maximum intersection time that is valid.
     *  @param rec a hit-record structure that will be populated with
     *      data about the intersection, if <code>ray</code> intersects
     *      this surface in the interval [t0, t1].  The only attribute
     *      that will not be filled in is the surface pointer itself.
     *      Clients should use 
     *      <code>sfc_hit()</code> instead of calling this function directly.
     *  @return <code>true</code> if <code>ray</code> intersects this surface
     *      in the interval [t0, t1], <code>false</code> otherwise.
     */
    bool (*hit_fn)(void* data, ray3_t* ray, float t0, float r1, 
            hit_record_t* rec) ;
    /** The diffuse color of this surface.
     */
    color_t*         diffuse_color ;
    /** The ambient color of this surface.
     */
    color_t*         ambient_color ;
    /** The specular color of this surface.
     */
    color_t*         spec_color ;
    /** the specular reflection color of this surface.
     */
    color_t*        refl_color ;
    /** The Phong exponent for this surface.
     */
    float           phong_exp ;
} ;

/** The structure representing a point light source.
 */
struct _light_t {
    /** The position of the light.
     */
    point3_t* position ;
    /** The color of the light.
     */
    color_t* color ;
} ;

/** The hit-record structure containing data about the intersection between
 *  a ray and a surface.
 */
struct _hit_record_t {
    /** The surface that was hit.
     */
    surface_t*  sfc ;
    /** The time the ray hits <code>sfc</code>; i.e., if the ray has
     *  the form e + sd for s &ge; 0, then the intersection point is
     *  e + td.
     */
    float   t ;
    /** The intersection point.
     */
    point3_t hit_pt ;
    /** The surface normal at <code>hit_pt</code>.
     */
    vector3_t normal ;
} ;

/** Create a sphere surface.  The specular reflection color is set to
 *  NULL; only if this is changed explicitly will specular reflections
 *  be calculated from this surface.
 *  
 *  @param x the x-coordinate of the center of the sphere.
 *  @param y the y-coordinate of the center of the sphere.
 *  @param z the z-coordinate of the center of the sphere.
 *  @param radius the radius of the sphere.
 *  @param diffuse_color the diffuse color of the sphere.
 *  @param ambient_color the ambient color of the sphere.
 *  @param spec_color the specular color of the sphere.
 *  @param phong_exp the Phong exponent of the sphere.
 *
 *  @return a <code>surface_t*</code> representing the sphere specified
 *      by the above data.
 */
surface_t* make_sphere(float x, float y, float z, float radius,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) ;

/** Create a triangle surface.  The specular reflection color is set to
 *  NULL; only if this is changed explicitly will specular reflections
 *  be calculated from this surface.  The surface normal will point
 *  in the direction of (b-a) x (c-a) (cross-product).
 *  
 *  @param a one vertex of the triangle.
 *  @param b one vertex of the triangle.
 *  @param c one vertex of the triangle.
 *  @param diff the diffuse color of the triangle.
 *  @param amb the ambient color of the triangle.
 *  @param spec the specular color of the triangle.
 *  @param phong_exp the Phong exponent of the triangle.
 *
 *  @return a <code>surface_t*</code> representing the triangle specified
 *      by the above data.
 */
surface_t* make_triangle(point3_t a, point3_t b, point3_t c,
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) ;

/** Create a plane surface.  The specular reflection color is set to
 *  NULL; only if this is changed explicitly will specular reflections
 *  be calculated from this surface.  The surface normal will point
 *  in the direction of (b-a) x (c-a) (cross-product).  The plane extends
 *  infinitely far in all directions.
 *  
 *  @param a one vertex of the triangle that defines the plane.
 *  @param b one vertex of the triangle that defines the plane.
 *  @param c one vertex of the triangle that defines the plane.
 *  @param diff the diffuse color of the plane.
 *  @param amb the ambient color of the plane.
 *  @param spec the specular color of the plane.
 *  @param phong_exp the Phong exponent of the plane.
 *
 *  @return a <code>surface_t*</code> representing the plane specified
 *      by the above data.
 */
surface_t* make_plane(point3_t a, point3_t b, point3_t c,
        color_t* diff, color_t* amb, color_t* spec, float phong_exp) ;

/** Determine whether a ray hits a surface in a specified interval;
 *  if so, fill in a hit-record with information about the intersection.
 *
 *  @param sfc the surface for which to check for intersection.
 *  @param ray the ray for which to check for intersection.
 *  @param t0 the minimum time for which to consider intersections valid.
 *  @param t1 the maximum time for which to consider intersections valid.
 *  @param rec a hit-record structure that will be populated with
 *      data about the intersection, if <code>ray</code> intersects
 *      this surface in the interval [t0, t1].
 *  @return <code>true</code> if <code>ray</code> intersects this surface
 *      in the interval [t0, t1], <code>false</code> otherwise.
 */
bool sfc_hit(surface_t* sfc, ray3_t* ray, float t0, float t1, 
        hit_record_t* rec) ;


#endif
