/** @file color.h Color type definitions and some constants for
 *  common colors.
 *  Professor Danner
 *  Computer Graphics 356
 *  Homework #2B
 *  Evan Carmi (WesID: 807136) and Carlo Francisco (WesID: 774066)
 *  ecarmi@wesleyan.edu and jfrancisco@wesleyan.edu
 *
 */

#ifndef COLOR_H
#define COLOR_H

/** The type representing an RGB color.
 */
typedef struct _color_t color_t ;

/** The structure for an RGB color.  Colors are represented by red, green,
 *  and blue components, where each component is a floating-point value
 *  in the range [0.0, 1.0].  We expose the structure here so that it is
 *  easy to access the color components directly.
 */
struct _color_t {
    /** The red component of the color.
     */
    float       red ;
    /** The green component of the color.
     */
    float       green ;
    /** The blue component of the color.
     */
    float       blue ;
} ;

#endif

