/** @file surface.h Surface types and functions.
 *
 *  @author N. Danner
 */

#ifndef SURFACE_H
#define SURFACE_H

#include <stdbool.h>

#include "color.h"
#include "geom356.h"
#include "list356.h"

/** The type of a hit record.  The structure is exposed below.
 */
typedef struct _hit_record_t hit_record_t ;

/** The type of an axis-aligned box.  This structure is exposed below.
 */
typedef struct _bbox_t bbox_t ;

/** The type of a surface.  This structure is exposed below.
 */
typedef struct _surface_t surface_t ;

/** The type of a point light source.  The structure is exposed below.
 */
typedef struct _light_t light_t ;

/** The axis-aligned box structure.  We expose its definition so as to
 *  make direct access to the components simpler.
 */
struct _bbox_t {
    /** Left cutting plane along x-axis.
     */
    float left ;
    /** Right cutting plane along x-axis.
     */
    float right ;
    /** Bottom cutting plane along y-axis.
     */
    float bottom ;
    /** Top cutting plane along y-axis.
     */
    float top ;
    /** Near cutting plane along z-axis.
     */
    float near ;
    /** Far cutting plane along z-axis.
     */
    float far ;
} ;

/** The surface structure.  We expose its definition so as to make direct
 *  access to the components simpler.
 */
struct _surface_t {
    /** Data that is specific to the type of surface.
     */
    void*           data ;

    /** An axis-aligned bounding box that encloses this surface.  Clients
     *  should not modify this field.
     */
    bbox_t* bbox ;

    /** The ray-surface intersection function for this surface.
     *  
     *  @param sfc a reference to the surface that will be recorded
     *      as the surface that was hit, if <code>hit_fn</code>
     *      returns <code>true</code>.
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
    bool (*hit_fn)(surface_t* sfc, ray3_t* ray, float t0, float r1, 
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

    /** The refraction index for this surface.  If <code>-1.0</code>,
     *  then this surface is not transparent.  Transparent surfaces
     *  should have this set to some value <code>&ge;1.0</code>.
     */
    float           refr_index ;

    /** Attenuation for transparent surfaces.  This gives the attenuation
     *  "color" for rays as they pass through transparent surfaces.  It
     *  is ignored unless <code>refr_index != -1</code>.
     */
    color_t*        atten ;
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
 *  infinitely far in all directions.  Plane surfaces have no bounding
 *  box (i.e., a <code>NULL</code> bounding box), so cannot be included
 *  in bounding-box tree nodes.
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

/* We are not doing triangulated surfaces right now.
 * DO NOT IMPLEMENT THIS FUNCTION.
surface_t* make_poly_surface(point3_t* vertices, int num_vertices,
        int* indices, int num_indices, GLfloat* xfrm,
        color_t* diffuse_color, color_t* ambient_color, color_t* spec_color,
        float phong_exp) ;
*/

/** Create a bounding-box tree node from a list of surfaces.  Any
 *  compound surface (like a BBT node) will <i>not</i> be broken into
 *  its component surfaces.
 *
 *  @param surfaces a list of surfaces.  Each surface in
 *      <code>surfaces</code> must have a non-<code>NULL</code>
 *      bounding box and must not be transparent.
 */
surface_t* make_bbt_node(list356_t* surfaces) ;

/** Determine whether a ray hits a surface in a specified interval;
 *  if so, fill in a hit-record with information about the intersection.
 *
 *  @param sfc the surface for which to check for intersection.
 *  @param ray the ray for which to check for intersection.
 *  @param t0 the minimum time for which to consider intersections valid.
 *  @param t1 the maximum time for which to consider intersections valid.
 *  @param rec a hit-record structure that will be populated with
 *      data about the intersection, if <code>ray</code> intersects
 *      this surface in the interval [t0, t1].  If <code>ray</code>
 *      does not intersect <code>sfc</code> in this interval, then
 *      <code>rec</code> will not be modified.
 *  @return <code>true</code> if <code>ray</code> intersects this surface
 *      in the interval [t0, t1], <code>false</code> otherwise.
 */
bool sfc_hit(surface_t* sfc, ray3_t* ray, float t0, float t1, 
        hit_record_t* rec) ;


#endif
