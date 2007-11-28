#ifndef RENORMALIZATION_H
#define RENORMALIZATION_H

#define MARGE 0.1

#define NODES 12
#define EDGE_12 1
#define EDGE_1A 2
#define EDGE_14 4
#define EDGE_23 8 
#define EDGE_2A 16
#define EDGE_2B 32
#define EDGE_3B 64
#define EDGE_35 128
#define EDGE_4A 256
#define EDGE_4C 512
#define EDGE_45 1024
#define EDGE_46 2048
#define EDGE_5B 4096
#define EDGE_5D 8192
#define EDGE_58 16384
#define EDGE_6C 32768
#define EDGE_67 65536
#define EDGE_68 131072
#define EDGE_7C 262144
#define EDGE_7D 524288
#define EDGE_8D 1048576
#define EDGE_AB 2097152
#define EDGE_AC 4194304
#define EDGE_AD 8388608
#define EDGE_BC 16777216
#define EDGE_BD 33554432
#define EDGE_CD 67108864

typedef struct {
    int node_start;
    int node_end;
} Edge;

#define CELL_TOPLEFT 1
#define CELL_TOPRIGHT 2
#define CELL_BOTTOMLEFT 4
#define CELL_BOTTOMRIGHT 8

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
    int trace[NODES];
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
