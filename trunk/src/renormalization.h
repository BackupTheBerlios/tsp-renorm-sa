#ifndef RENORMALIZATION_H
#define RENORMALIZATION_H

#include "block.h"
#define MARGE 0.1

#define BIT_CELL_TL 1
#define BIT_CELL_TR 2
#define BIT_CELL_BL 4
#define BIT_CELL_BR 8
#define BIT_CELL_MAX 16

#define CELL_NODES 4
#define BORDER_NODES 8
#define CROSS_NODES 5
#define NORMAL_NODES CELL_NODES + BORDER_NODES
#define NODES CELL_NODES + BORDER_NODES + CROSS_NODES

/*
 * T = Top
 * C = Center
 * L = Left
 * R = Right
 * B = Bottom
 * TL = Top Left
 * TR = Top Right
 * BL = Bottom Left
 * BR = Bottom Right
 */
#define NODE_CELL_TL 0
#define NODE_CELL_TR 1
#define NODE_CELL_BL 2
#define NODE_CELL_BR 3
#define ANY_NODE_CELL 4

#define NODE_BORDER_TL 4
#define NODE_BORDER_T 5
#define NODE_BORDER_TR 6
#define NODE_BORDER_L 7
#define NODE_BORDER_R 8
#define NODE_BORDER_BL 9
#define NODE_BORDER_B 10
#define NODE_BORDER_BR 11

#define NODE_CROSS_T 12
#define NODE_CROSS_L 13
#define NODE_CROSS_C 14
#define NODE_CROSS_R 15
#define NODE_CROSS_B 16

/*
 * Basic two by two cell. This cell has three types of points. These points are:
 * - Border points: points on the border of the two by two cell
 * - Cross points: Points internal in the basic cell where an edge crosses the 
 *                 internal border
 * 
 * The basic cell is divided into four cells: NODE_CELL_TL, NODE_CELL_TR, 
 *                                            NODE_CELL_BL and NODE_CELL_BR
 * 
 * In a picture you can see this as follows:
 * 
 *  [TL]------------[T]------------[TR]
 *  |                |              |
 *  |     (TL)      {T}    (TR)     | 
 *  |                |              |
 *  [L]---{L}-------{C}----{R}-----[R]
 *  |                |              |
 *  |     (BL)      {B}    (BR)     | 
 *  |                |              |
 *  [BL]------------[B]------------[BR]
 * 
 * Here [..] is a border point, {..} is a cross point and (..) is the node at 
 * the center of a cell
 *
 * The Cell has the following information stored:
 *  - Trace is an optimal route in the node, where each visited point is stored
 *  - Trace_length is the length of this trace
 *  - Length is the length of the optimal tour
 *  - start and end are array where the reference points of the basic cells are 
 *    stored (The start and endpoint) 
 */
typedef struct
{
   int     trace[NODES];
   int     trace_length;

   double  length;

   int     visits[CELL_NODES];
   int     start[CELL_NODES];
   int     end[CELL_NODES];
} Route;

typedef struct
{
   Route **routes;
   int     size;
} Route_array;

typedef struct
{
   Route  *route;
   int     x;
   int     y;
} Block;

/*
 * Function which solves the Symmetric TSP by using renormalization technique
 */
int    *renormalize();

/*
 * Get the basic route. A basic route is a case where no entry point and 
 * departure point are specified on the edge of the square. For each cell
 * is specified if it needs to be visited or not using the arguments. 
 * The basic route is a closed path connecting the selected points
 */
Route  *get_basic_route(int cells);

/*
 * Calculate all shortest routes for the default graph(See top of this file). 
 * Here difference is made for the cells which are visited. For all 
 * the nodes shortest paths are calculated for every possible combination of 
 * cells which need to be visited. The visited cells are encoded using bitmasks,
 * for making the iteration more simple.
 */
void    preprocess_routes();
void    free_routes();
/*
 * Finds entry and departure blocks in one route block 
 */
void    set_borderpoints_subblocks(Route * route);
/*
 * Function giving all possible paths between start and end, which visit each 
 * node maximal one time. It results a list of pointers to the found routes
 */
Route_array paths(int start, int end, int visited);

/*
 * Add a route to a route array. 
 * Herefore the allocated size of the array is increased
 */
void    add_route(Route_array * array, Route * route);

/*
 *  Copy a route
 */
Route  *copy_route(Route * src);

/*
 * Check if a specific node is in the route
 */
int     node_in_route(int node, Route * route);

/*
 * Check if the route is valid and visits the cells specified in 
 * the bitmask <cells>
 */
int     route_visits_cells(Route * route, int cells);

void    make_weight_matrix();
int     bitmask(grd * grid, int ind_x, int ind_y);
int     convert_node(int location, int global_point);
void    get_corresponding_cell(int point, int *cell_a, int *cell_b);
void    get_cell_index(Route * route, int start, int end, int *cell_a,
                       int *cell_b);
void    print_routes(Block * blocks, int size, FILE * f);
void    map_block_on_route(Block * block, grd * grid, int *ind);

#endif
