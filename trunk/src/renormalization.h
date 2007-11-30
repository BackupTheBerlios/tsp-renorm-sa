#ifndef RENORMALIZATION_H
#define RENORMALIZATION_H

#define MARGE 0.1

#define CELL_TOPLEFT 1
#define CELL_TOPRIGHT 2
#define CELL_BOTTOMLEFT 4
#define CELL_BOTTOMRIGHT 8

#define MAX_CELLS 16

#define BORDER_NODES 8
#define NODES 12

#define NODE_TOPLEFT 8
#define NODE_TOPRIGHT 9
#define NODE_BOTTOMLEFT 10
#define NODE_BOTTOMRIGHT 11


/*
 Struct for computing now shortest paths on the two by two blocks
  - Trace are the visited nodes [0-11] 
  - Trace_length number of nodes
  - Length is the length of the tour
  - Visits are the visited centrum points(Bit mask) */
typedef struct {
    int trace[NODES + 6];
    int trace_length;
    double length;
} Route;

typedef struct {
    Route** routes;
    int size;
} Route_array;

void renormalize(Tsp *tsp);

Route* getBasicRoute(int cell_topleft, int cell_topright,
                     int cell_bottomleft, int cell_bottomright);

                     
void preprocess_routes();
Route_array paths(int start, int end, int visited);

void make_weight_matrix();

void add_route(Route_array *array, Route *route);
void copy_route(Route* dest, Route* src);

int route_visits_cells(Route* route, int cells);
int node_in_route(int node, Route *route);


#endif
