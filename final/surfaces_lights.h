/** @file Create a lists of surfaces and lights for use by the ray tracer.
 */

#ifndef OBJECTS_LIGHTS_H
#define OBJEcTS_LIGHTS_H

#include "list356.h"
#include "geom356.h"

/** Get a list of surfaces.  Each element of the list will be of type
 *  <code>surface_t*</code>.
 *
 *  @return a list of surfaces to render.
 */
list356_t* get_surfaces() ;

/** Get a list of lights.  Each element of the list will be of type
 *  <code>light_t*</code>.
 *
 *  @return a list of lights.
 */
list356_t* get_lights() ;

/** Get the ambient light color.
 *  
 *  @param ambient_light a structure to fill with the ambient light
 *      color.
 */
void get_ambient_light(color_t* ambient_light) ;

/** Get the viewing data.  Given a location for the viewpoint and look-at
 *  point and an up-direction in the world-frame, the standard 
 *  orthonormal camera frame
 *  will be computed with origin at the viewpoint, <i>w</i> pointing
 *  from the viewpoint away from the look-at point, and <i>v</i> in the
 *  same plane as <i>w</i> and the up-direction.  The basis
 *  <i>(u, v, w)</i> will be a right-handed frame.
 *
 *  @param eye the object that will be filled with the coordinates of the
 *      viewpoint.
 *  @param look_at the object that will be filled with the coordinates
 *      of the look-at point.
 *  @param up the object that will be filled with the coordinates of the
 *      world-frame up direction.
 */
void set_view_data(point3_t* eye, point3_t* look_at, vector3_t* up) ;

/** Get the viewing rectangle data.  After invoking this function, the
 *  caller will have the distance from the viewpoint to the viewing
 *  rectangle and the width and height of the rectangle.  The viewing
 *  rectangle should then be defined as the rectangle that lies in
 *  the plane that is parallel to the the camera frame <i>u-v</i> plane
 *  and such that <i>w intersects the viewing rectangle in its center.
 */
void set_view_plane(float* dist, float* width, float* height) ;

#endif
