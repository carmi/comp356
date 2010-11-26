/** @file maze.h structures and functions for mazes.
 *  
 *  A maze is a two-dimensional grid of cells that are identified by
 *  a row and column position (0-indexed).  <code>cell_t</code> objects
 *  represent cells; such objects should only ever be obtained by invoking
 *  functions defined here.
 *
 *  @author N. Danner.
 */

#ifndef MAZE_H
#define MAZE_H

#include <stdbool.h>

/** The type of a cell.  Cells should not be created directly; only
 *  use <code>get_cell</code>.  Failure to do so may lead to unpredictable
 *  results.
 */
struct _cell_t {
    int r ;
    int c ;
} ;
typedef struct _cell_t cell_t ;

/** The type of a maze.
 */
typedef struct _maze_t maze_t ;

// Directions
#define NORTH 0x1
#define EAST 0x2
#define SOUTH 0x04
#define WEST 0x08

/** Test two cells for equality.
 *  
 *  @param x one cell.
 *  @param y another cell.
 *
 *  @return 0 if <code>x</code> and <code>y</code> represent the same cell,
 *      a non-zero number otherwise.
 */
int cell_cmp(void* x, void* y) ;

/** Get a cell from a maze for a given row and column.
 *  
 *  @param m the maze.
 *  @param r the row of the desired cell.
 *  @param c the column of the desired cell.
 *
 *  @return a <code>cell_t</code> object corresponding to <code>r</code> and
 *  <code>c</code>.
 */
cell_t* get_cell(maze_t* m, int r, int c) ;

/** Make a maze of a given size.  The maze will initially have all walls
 *  present; then Prim's algorithm will be used to remove walls so that
 *  there is exactly one path between any two cells in the maze.  Prim's
 *  algorithm will choose walls to remove from the frontier at random,
 *  and the random number generator will be seeded with <code>seed</code>.
 *
 *  @param nrows the number of rows for the maze.
 *  @param ncols the number of columns for the maze.
 *  @param seed the seed for the random number generator used by Prim's
 *      algorithm to choose walls to remove.
 *
 *  @return the maze.
 */
maze_t* make_maze(int nrows, int ncols, long seed) ;

/** Get the start cell of a maze.
 *  
 *  @param m a maze.
 *  @return the start cell of <code>m</code>.
 */
cell_t* get_start(maze_t* m) ;

/** Get the end cell of a maze.
 *  
 *  @param m a maze.
 *  @return the end cell of <code>m</code>.
 */
cell_t* get_end(maze_t* m) ;

/** Get the number of rows of a maze.
 *
 *  @param m a maze.
 *  @return the number of rows in <code>m</code>.
 */
int get_nrows(maze_t* m) ;

/** Get the number of columns of a maze.
 *
 *  @param m a maze.
 *  @return the number of columns in <code>m</code>.
 */
int get_ncols(maze_t* m) ;

/** Check whether there is a passage in a given direction from a given cell
 *  in a maze.
 *
 *  @param m a maze.
 *  @param c a cell in <code>m</code>.
 *  @param d a direction.
 *
 *  @return <code>true</code> if there is a passage in direction <code>d</code>
 *      from <code>c</code> in <code>m</code>, <code>false</code> otherwise.
 */
bool has_path(maze_t* m, cell_t* c, unsigned char d) ;

/** Check whether there is a wall in a given direction from a given cell
 *  in a maze.
 *
 *  @param m a maze.
 *  @param c a cell in <code>m</code>.
 *  @param d a direction.
 *
 *  @return <code>true</code> if there is a wall in direction <code>d</code>
 *      from <code>c</code> in <code>m</code>, <code>false</code> otherwise.
 */
bool has_wall(maze_t* m, cell_t* c, unsigned char d) ;

#endif

