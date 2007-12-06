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
 Struct for computing now shortest paths on the two by two blocks
  - Trace are the visited nodes [0-11] 
  - Trace_length number of nodes
  - Length is the length of the tour
  - Visits are the visited centrum points(Bit mask) */
typedef struct {
    int trace[NODES];
    int trace_length;
    
    double length;
    
    int visits[CELL_NODES];
    int start[CELL_NODES];
    int end[CELL_NODES];    
} Route;

typedef struct {
    Route** routes;
    int size;
} Route_array;

typedef struct {
    Route* route;
    int x;
    int y;
} Block;

int* renormalize();
void map_block_on_route(Block* block, grd *grid, int *ind);

Route* get_basic_route(int cells);

                     
void preprocess_routes();
void free_routes();
void set_borderpoints_subblocks(Route* route);

Route_array paths(int start, int end, int visited);

void make_weight_matrix();
int bitmask(grd *grid, int ind_x, int ind_y);

void add_route(Route_array *array, Route *route);
Route* copy_route(Route* src);

int node_in_route(int node, Route *route);
int convert_node(int location, int global_point);

int route_visits_cells(Route* route, int cells);
void get_corresponding_cell(int point, int *cell_a, int *cell_b);
void get_cell_index(Route* route, int start, int end, int *cell_a, int *cell_b);

void print_routes(Route*** routes, int cells_x, int cells_y, FILE *f);
#endif
