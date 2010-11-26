/** maze.c:  structures and functions for mazes.
 *
 *  @author N. Danner
 */

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include "stdlib.h"
#include <time.h>

#include "list356.h"

#include "debug.h"
#include "maze.h"

// Directions.
#define EMPTY 0x0
unsigned char directions[] = {NORTH, EAST, SOUTH, WEST} ;

// Offset into the maze cells based on row and column position.
#define CELL(m, r, c) (*(m->cells + (r*(m->ncols)) + c))

// Offset into the maze cells based on a cell_t.
#define MAZECELL(m, cc) (*(m->cells + (cc->r)*(m->ncols) + (cc->c)))

/** Type of an edge between two cells, for use in Prim's algorithm.
 */
typedef struct _edge_t {
    cell_t* a ;
    cell_t* b ;
} edge_t ;

/** Type of a maze.
 */
struct _maze_t {
    /** Each cell is a bitmask of directions; for a cell c, if c&d != 0
     *  for a direction d, then there is a passage (no wall) in the direction d.
     */
    unsigned char *cells ;

    /** Number of rows and columns for this maze.
     */
    int nrows, ncols ;

    /** The start and end cells of the maze.
     */
    cell_t *start, *end ;
} ;

/** Build a maze using Prim's algorithm.  After calling, walls will have
 *  been removed from <code>maze</code> to ensure that there is exactly
 *  one path between any pair of cells.
 *  
 *  @param maze the maze.  All walls must be present.
 */
static void build_prim(maze_t* maze) ;

// CELL AND EDGE FUNCTIONS.

/** The cell objects.  We do our own "memory management" for cells by 
 *  maintaining a 2D array of cell_t objects; use get_cell to get the
 *  appropriate object given a row and column.
 */
cell_t* cells = NULL ;;

/** Get a cell given row and column; see maze.h.
 */
cell_t* get_cell(maze_t* m, int r, int c) {
    return &cells[r*m->ncols+c] ;
}

/** Test two cells for equality; see maze.h.
 */
int cell_cmp(void* c, void* d) {
    return c == d ? 0 : 1 ;
}

/** Test two edges for equality.
 *
 *  @param x1 one edge
 *  @param x2 another edge
 *
 *  @return 0 if the two edges are equal (i.e., have the same cells
 *      as endpoints in either order), non-0 otherwise.
 */
static int edge_cmp(void* x1, void* x2) {
    edge_t* e1 = x1 ;
    edge_t* e2 = x2 ;
    return (cell_cmp(e1->a, e2->a) + cell_cmp(e1->b, e2->b))*
        (cell_cmp(e1->a, e2->b) + cell_cmp(e1->b, e2->a)) ;
}

/** Get a random integer within a given range.
 *
 *  @param lower the lower limit of the range.
 *  @param upper the upper limit of the range.
 *
 *  @return a random number in [lower, upper).
 */
static int random_limit(int lower, int upper) {
    int val = lower+random()%(upper-lower) ;
    return val ;
}

/** Make a maze.  See maze.h.
 */
maze_t* make_maze(int nrows, int ncols, long seed) {

    // Allocate the array of cell objects.
    if (cells != NULL) free(cells) ;
    cells = malloc(nrows*ncols*sizeof(cell_t)) ;
    for (int r=0; r<nrows; ++r) {
        for (int c=0; c<ncols; ++c) {
            cells[r*ncols+c].r = r ;
            cells[r*ncols+c].c = c ;
        }
    }

    srandom(seed) ;

    // Choose start and end cells at random, ensuring that they are not the
    // same cell.
    maze_t* m = malloc(sizeof(maze_t)) ;
    m->cells = malloc(nrows*ncols*sizeof(unsigned char)) ;
    m->nrows = nrows ;
    m->ncols = ncols ;
    m->start = get_cell(m, random_limit(0, nrows), random_limit(0, ncols)) ;
    cell_t* end_cell ;
    do {
        end_cell = get_cell(m, random_limit(0, nrows), random_limit(0, ncols)) ;
    } while (end_cell == m->start) ;
    m->end = end_cell ;

    // Add all possible walls to the maze.
    for (int r=0; r<nrows; ++r) {
        for (int c=0; c<ncols; ++c) {
            CELL(m, r, c) = EMPTY ;
        }
    }

    // Generate the maze.
    build_prim(m) ;

    return m ;
}

/** Get the start cell of a maze; see maze.h.
 */
cell_t* get_start(maze_t* m) {
    return m->start ;
}

/** Get the end cell of a maze; see maze.h.
 */
cell_t* get_end(maze_t* m) {
    return m->end ;
}

/** Get the number of rows of a maze; see maze.h.
 */
int get_nrows(maze_t* m) {
    return m->nrows ;
}

/** Get the number of columns of a maze; see maze.h.
 */
int get_ncols(maze_t* m) {
    return m->ncols ;
}

/** Check for a path to an adjacent cell; see maze.h.
 */
bool has_path(maze_t* m, cell_t* c, unsigned char d) {
    return (MAZECELL(m, c) & d) != 0 ;
}

/** Check for a path to an adjacent cell; see maze.h.
 */
bool has_wall(maze_t* m, cell_t* c, unsigned char d) {
    return !has_path(m, c, d) ;
}

/** Get the cell in a given direction from a given cell.
 *  
 *  @param cell the given cell.
 *  @param direction the direction
 *
 *  @return the cell that is adjacent to <code>cell</code> in direction 
 *      <code>direction</code>.
 */
static cell_t* get_neighbor(maze_t* m, cell_t* cell, unsigned char direction) {
    switch (direction) {
        case NORTH:
            return get_cell(m, 1+cell->r, cell->c) ;
        case EAST:
            return get_cell(m, cell->r, 1+cell->c) ;
        case SOUTH:
            return get_cell(m, cell->r-1, cell->c) ;
        case WEST:
            return get_cell(m, cell->r, cell->c-1) ;
        default:
            assert(0) ;
    }
    assert(0) ;
    
    // Kill "control reaches end" compiler warnings with -DNDEBUG.
    return NULL ;
}

/** Get the direction from one cell to another.
 *  
 *  @param a a cell.
 *  @param b a cell that is adjacent to a.
 *
 *  @return the direction from a to b.
 */
static unsigned char get_direction(cell_t* a, cell_t* b) {
    if (b->r == a->r+1) return NORTH ;
    if (b->r == a->r-1) return SOUTH ;
    if (b->c == a->c+1) return EAST ;
    if (b->c == a->c-1) return WEST ;
    assert(0) ;
    
    // Kill "control reaches end" compiler warnings with -DNDEBUG.
    return 0 ;
}

/** Get the opposite direction from a given direction.
 *  
 *  @param direction any direction.
 *
 *  @return the opposite direction from <code>direction</code>.
 */
static unsigned char opposite(unsigned char direction) {
    switch (direction) {
        case NORTH:
            return SOUTH ;
        case EAST:
            return WEST ;
        case SOUTH:
            return NORTH; 
        case WEST:
            return EAST ;
    }
    assert(0) ;
    
    // Kill "control reaches end" compiler warnings with -DNDEBUG.
    return 0 ;
}

/** Check whether there is a cell in the given direction from a given
 *  cell.
 *
 *  @param m a maze.
 *  @param c a cell in <code>m</code>.
 *  @param d a direction.
 *
 *  @return <code>true</code> if there is a cell in direction <code>d</code>
 *      from <code>c</code>, <code>false</code> otherwise.
 */
static bool is_cell(maze_t* m, cell_t* c, unsigned char d) {
    switch(d) {
        case NORTH:
            return (c->r < m->nrows-1) ;
        case EAST:
            return (c->c < m->ncols-1) ;
        case SOUTH:
            return (c->r >= 1) ;
        case WEST:
            return (c->c >= 1) ;
    }
    assert(0) ;
    
    // Kill "control reaches end" compiler warnings with -DNDEBUG.
    return false ;
}

/** Remove a wall from the maze.
 *  
 *  @param m a maze.
 *  @param c a cell in <code>m</code>.
 *  @param d a direction.
 */
static void remove_wall(maze_t* m, cell_t* c, unsigned char d) {
    MAZECELL(m, c) |= d ;
    cell_t* adj = get_neighbor(m, c, d) ;
    MAZECELL(m, adj) |= opposite(d) ;

}

/** Build the maze by removing walls according to Prim's algorithm.
 *
 *  @param maze a maze with all walls present.
 */
static void build_prim(maze_t* maze) {
    // MST edges and cells.  If (a, b) in mst_edges, then (b, a) not in
    // mst_edges.  (a, b) in mst_edges iff a, b both in mst_cells.
    list356_t* mst_edges = make_list() ;
    list356_t* mst_cells = make_list() ;

    // The frontier.  This is the collection of edges between cells in the MST
    // and cells not in the MST.  If (a, b) in frontier, then a in mst_cells
    // and b not in mst_cells.
    list356_t* frontier = make_list() ;

    // Choose two adjacent cells at random to put into the MST, then
    // populate the frontier accordinately.  For simplicitly, choose a
    // cell in the interior of the maze, then randomly choose a direction
    // for the other cell.
    cell_t* start = 
        get_cell(maze, random_limit(1, maze->nrows-1), 
                random_limit(1, maze->ncols-1));
    unsigned char direction = 1 << random_limit(0, 4) ;

    cell_t* next = get_neighbor(maze, start, direction) ;
    /*
    debug("Removing (%d, %d) - (%d, %d).\n",
            start->r, start->c, next->r, next->c) ;
    */
    remove_wall(maze, start, direction) ;

    edge_t init_edge = (edge_t){start, next} ;
    lst_add(mst_edges, &init_edge) ;
    lst_add(mst_cells, start) ;
    lst_add(mst_cells, next) ;

    for (int d=0; d<4; ++d) {
        if (directions[d] != direction && is_cell(maze, start, directions[d])) {
            cell_t* c = get_neighbor(maze, start, directions[d]) ;
            edge_t* e = malloc(sizeof(edge_t)) ;
            e->a = start ; e->b = c ;
            lst_add(frontier, e) ;
        }
    }

    for (int d=0; d<4; ++d) {
        if (directions[d] != opposite(direction) 
                && is_cell(maze, next, directions[d])) {
            cell_t* c = get_neighbor(maze, next, directions[d]) ;
            edge_t* e = malloc(sizeof(edge_t)) ;
            e->a = next ; e->b = c ;
            lst_add(frontier, e) ;
        }
    }

    // As long as we don't have all the cells in the MST, choose an
    // edge in the frontier at random.  Put the edge in the MST
    // and compute the new edges to add to the frontier.
    while (lst_size(mst_cells) < (maze->nrows)*(maze->ncols)) {
        int p = random_limit(0, lst_size(frontier)) ;
        edge_t* edge = lst_get(frontier, p) ;
        cell_t* old_cell = edge->a ;
        cell_t* new_cell = edge->b ;
        /*
        debug("Removing (%d, %d) - (%d, %d).\n",
                old_cell->r, old_cell->c, new_cell->r, new_cell->c) ;
        */
        remove_wall(maze, old_cell, get_direction(old_cell, new_cell)) ;

        lst_add(mst_edges, edge) ;
        lst_add(mst_cells, new_cell) ;

        for (int d=0; d<4; ++d) {
            unsigned char dir = directions[d] ;
            if (is_cell(maze, new_cell, dir)) {
                cell_t* adj_cell = get_neighbor(maze, new_cell, dir) ;
                edge_t* edge2 = malloc(sizeof(edge_t)) ;
                edge2->a = new_cell ; edge2->b = adj_cell ;
                if (lst_contains(mst_cells, adj_cell, cell_cmp)) {
                    lst_remove(frontier, edge2, edge_cmp) ;
                    if (adj_cell != old_cell) free(edge2) ;
                }
                else {
                    lst_add(frontier, edge2) ;
                }
            }
        }
    }


}

