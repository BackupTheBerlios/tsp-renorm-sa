#include <assert.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>

#include "tsp.h"
#include "renormalization.h"
#include "io.h"

/*
  Weights of edges between nodes on the default block.
  The default block is:

  (0)------------(1)------------(2)
  |               |              |
  |     (8)       |      (9)     | 
  |               |              |
  (3)---------------------------(4)
  |               |              |
  |     (10)      |     (11)     | 
  |               |              |
  (5)------------(6)------------(7)

  The weights of the possible edges are 0 if there is no edge, 
  and non zero if there is one
*/
static double _weights_0[] = {0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.0, 0.0};
static double _weights_1[] = {1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.707, 0.707, 0.0, 0.0};
static double _weights_2[] = {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.0};
static double _weights_3[] = {1.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.0};
static double _weights_4[] = {0.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.707, 0.0, 0.707};
static double _weights_5[] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.707, 0.0};
static double _weights_6[] = {0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.707, 0.707};
static double _weights_7[] = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707};
static double _weights_8[] = {0.707, 0.707, 0.0, 0.707, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.414};
static double _weights_9[] = {0.0, 0.707, 0.707, 0.0, 0.707, 0.0, 0.0, 0.0, 1.0, 0.0, 1.414, 1.0};
static double _weights_10[] = {0.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.707, 0.0, 1.0, 1.414, 0.0, 1.0};
static double _weights_11[] = {0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.707, 1.141, 1.0, 1.0, 0.0};

static double *_weights[12];

static Route _shortest_routes[9][9][16];

/*
  * Function which solves the Symmetric TSP by using renormalization technique
  */
void renormalize(Tsp *tsp)
{
    int cells_x, cells_y;
    int i, x, y, unity;
    double min_x, min_y;
    double max_x, max_y;
    double range_x, range_y;
    int ind_x, ind_y;
    int **cells = NULL;
    Route ***route_new = NULL;
    Route ***route_prev = NULL;
    
    /* Set up grid, only for testing purposes, should call help function */
    cells_x = 2;
    cells_y = 2;
    
    min_x = max_x = tsp->cities[0].x;
    min_y = max_y = tsp->cities[0].y;
    for (i = 1; i < tsp->dimension; i++) {
        if (tsp->cities[i].x < min_x)
            min_x = tsp->cities[i].x;
        if (tsp->cities[i].x > max_x)
            max_x = tsp->cities[i].x;
        if (tsp->cities[i].y < min_y)
            min_y = tsp->cities[i].y;
        if (tsp->cities[i].y < max_y)
            min_x = tsp->cities[i].y;
    }
    min_x -= MARGE;
    max_x += MARGE;
    min_y -= MARGE;
    max_y += MARGE;
    
    /*
             * Renormalize space until unity is reached. The renormalization procedure decreases the scale of the grid used for
             * approximating the shortest route. The space is divided into cells, the smaller the scale, the more cells. For each cell 
             * there is checked if a city is within the cell. Now for each block consisting of four cells the route is filled in. This 
             * route is optimal and should already be calculated by preprocess_routes. The previous iteration decides
             * the entry and departure place of the block.
             */
    unity = 0;
    while (!unity) {
        /* Generate Cartesian grid, should also be done by a help function (here for testing purposes) */
        if ((cells= realloc(cells, cells_x * sizeof(int*))) == NULL) 
            errx(EX_OSERR, "Out of memory");
        
        for (i = 0; i < cells_x; i++) 
            if ((cells[i] = realloc(cells[i], cells_y * sizeof(int))) == NULL) 
                errx(EX_OSERR, "Out of memory");
        

        for (x = 0; x < cells_x; x++)
            for (y = 0; y < cells_y; y++)
                cells[x][y] = 0;
        
        range_x = (max_x - min_x) / cells_x;
        range_y = (max_y - min_y) / cells_y;
        
        /* Check if a city is within a cell, should also be done by a help function (here for testing purposes) */
        unity = 1;
        for (i = 0; i < tsp->dimension; i++) {
            ind_x = (int)floor(tsp->cities[i].x - min_x / range_x);
            ind_y = (int)floor(tsp->cities[i].y - min_y / range_y);
            
            if (cells[ind_x][ind_y])
                unity = 0;
                
            cells[ind_x][ind_y] = 1;
        }
        
        /* Reserving space for the new route through the representative points*/
        if ((route_new = calloc(cells_x / 2, sizeof(Route**))) == NULL)
            errx(EX_OSERR, "Out of memory");
        
        /* For each block of four cells decide the shortest route which visits the centers of the cell where at least one
                       * city is located
                       */
        for (x = 0; x < cells_x  / 2; x++) {
            if ((route_new[x] = calloc(cells_y / 2, sizeof(Route*))) == NULL)
                errx(EX_OSERR, "Out of memory");
            for (y = 0; y < cells_y / 2; y++) {
                /* It is the first iteration, so entry and deperature points in a block are not an issue yet */
                if (!route_prev) {
                    route_new[x][y] = 
                      getBasicRoute(cells[x * 2][y * 2],
                               cells[x * 2][y * 2 + 1],
                               cells[x * 2 + 1][y * 2],
                               cells[x * 2 + 1][y * 2 + 1]);
                }
                else {
                
                }
            }
        }
    }
}

/*
  * Get the basic route. A basic route is a case where no entry point and  departure point are specified on the edge of the
  * square. For each cell is specified if it needs to be visited or not using the arguments. The basic route is a closed path
  * connecting the selected points
  */
Route* getBasicRoute(int cell_topleft, int cell_topright,
                     int cell_bottomleft, int cell_bottomright)
{
    Route* route;
    
    if ((route = calloc(1, sizeof(Route))) == NULL)
        errx(EX_OSERR, "Out of memory");
    
    /* Simply visit all cells and you have already the shortest path */
    if (cell_topleft) {
        route->trace[route->trace_length] = NODE_TOPLEFT;
        route->trace_length++;
    }
    if (cell_topright) {
        route->trace[route->trace_length] = NODE_TOPRIGHT;
        route->trace_length++;
    }
    if (cell_bottomleft) {
        route->trace[route->trace_length] = NODE_BOTTOMLEFT;
        route->trace_length++;
    }
    if (cell_bottomright) {
        route->trace[route->trace_length] = NODE_BOTTOMRIGHT;
        route->trace_length++;
    }
    
    /*Repeat last node to close the path (If the path is longer than two)*/
    if (route->trace_length > 2) {
        route->trace[route->trace_length] = route->trace[0];
        route->trace_length++;
    }
    
    return route;
}

/*
  * Calculate all shortest routes for the default graph(See top of this file). Here difference is made for the cells which are
  * visited. For all the nodes shortest paths are calculated for every possible combination of cells which need to be visited
  * The visited cells are encoded using bitmasks, for making the iteration more simple.
 */
void preprocess_routes()
{
    int cells;
    int start, end;
    int i, j;
    int paths_possible;
    int visit_topleft, visit_topright;
    int visit_bottomleft, visit_bottomright;
    int is_shorter;
    Route_array path;
    Route* shortest;
    
    make_weight_matrix();
    
    /*Iterate over all start and endpoints in the graph */
    for (start = 0; start < 9; start++) {
        for (end = 0; end < 9; end++) {
            path = paths(start, end, 0);

            /*Decide for each possible combination of visited cells, what the shortest route is*/
            for (cells = 0; cells < 16; cells++) {
                for (i = 0; i < path.size; i++) {
                    if (route_visits_cells(path.routes[i], cells)) {
                        if (shortest == NULL)
                            shortest = path.routes[i];
                        else if (path.routes[i]->length < shortest->length)
                            shortest = path.routes[i];
                    }

                    assert(shortest);
                    copy_route(&_shortest_routes[start][end][cells], shortest);
                }
            }
            free(path.routes);
        }
    }
}

/*
  * Check if the route is valid and visits the cells specified in the bitmask <cells>
  */
int route_visits_cells(Route* route, int cells)
{
    if (CELL_TOPLEFT & cells && !node_in_route(NODE_TOPLEFT, route))
        return 0;

    if (CELL_TOPRIGHT & cells && !node_in_route(NODE_TOPRIGHT, route))
        return 0;

    if (CELL_BOTTOMLEFT & cells && !node_in_route(NODE_BOTTOMLEFT, route))
        return 0;

    if (CELL_BOTTOMRIGHT & cells && !node_in_route(NODE_BOTTOMRIGHT, route))
        return 0;
    
    return 1;
}

/*
  *  Copy a route
  */
void copy_route(Route* dest, Route* src)
{
    int i;
    
    for (i = 0; i < NODES; i++)
        dest->trace[i] = src->trace[i];
    dest->trace_length = src->trace_length;
    dest->length = src->length;
}

/*
  Function giving all possible paths between start and end, which visit each 
  node maximal one time. It results a list of pointers to the found routes
 */
Route_array paths(int start, int end, int visited)
{
    Route_array routes_new;
    Route_array routes_cur;
    Route* route;
    int paths_previous_no, i, j;
    
    routes_new.routes = NULL;
    routes_new.size = 0;

    /* Function wants all the routes with the same start and endpoint for calculating the shortest path, 
            * Normally you would aspect that this path has a length of 0 (Stay at the location). However maybe he needs to
            * visit some cells
             */
    if (start == end && visited == 0) {
        /*Find edges out of the start point. Use the destination of these edge as a start point for a route to
                        *our endpoint, refuse routes which use the same edge we have selected, since it is not allowed to use 
                        *an edge twice
                        */
        for (i = 0; i < NODES; i++) {
            if (_weights[i][end] && !(visited & (1 << i))) {
                routes_cur = paths(i, start, 0);
        
                for (j = 0; j < routes_cur.size; j++) {
                    route = routes_cur.routes[j];
                    
                    /* Route is one edge long, so it uses this edge */
                    if (route->trace_length == 2) {
                        free(route);
                        continue;
                    }
                    
                    route->trace[route->trace_length] = end;
                    route->trace_length++;
                    route->length += _weights[i][end]; 
            
                    add_route(&routes_new, route);
                }
                free(routes_cur.routes);
            }
        }
    }
    
    /* Base case: all paths from a point to itself. Since we are eventually interested in shortest paths, 
            * there is actually one path (stay at your location)
             */
    else if (start == end) {
        if ((route = calloc(1, sizeof(Route))) == NULL)
            errx(EX_OSERR, "Out of memory");
        
        route->trace[0] = start;
        route->trace_length = 1;
        route->length = 0.0;
        add_route(&routes_new, route);
    }
    
    /* For getting all possible paths you search all connected neighbours. From here the paths are recursively determined
            * Now you only need to add this current point to the trace. Do not visit neighbours which are already visited in the current
            * route, since this would produce cycles in the route
            */
    else {
        for (i = 0; i < NODES; i++) {
            if (_weights[i][end] && !(visited & (1 << i))) {
                routes_cur = paths(start, i, visited | (1 << end));
        
                for (j = 0; j < routes_cur.size; j++) {
                    
                    route = routes_cur.routes[j];
                    route->trace[route->trace_length] = end;
                    route->trace_length++;
                    route->length += _weights[i][end]; 
            
                    add_route(&routes_new, route);
                }
                free(routes_cur.routes);
            }
        }
    }
    return routes_new;
}

/*
  * Add a route to a route array. Herefore the allocated size of the array is increased
  */
void add_route(Route_array *array, Route *route)
{
    assert(array);
    
    array->size++;
    array->routes = realloc(array->routes, array->size * sizeof(Route*));
    
    if (array->routes == NULL)
        errx(EX_OSERR, "Out of memory");
    
    array->routes[array->size - 1] = route;
}

/*
  * Check if a specific node is in the route
  */
int node_in_route(int node, Route *route)
{
    int i;

    for (i = 0; i < route->trace_length; i++)
        if (route->trace[i] == node)
            return 1;

    return 0;
}

/*
  * Initialize weight matrix of the default graph
  * Function is needed because this can not be done statically
  */
void make_weight_matrix()
{
    _weights[0] = _weights_0;
    _weights[1] = _weights_1;
    _weights[2] = _weights_2;
    _weights[3] = _weights_3;
    _weights[4] = _weights_4;
    _weights[5] = _weights_5;
    _weights[6] = _weights_6;
    _weights[7] = _weights_7;
    _weights[8] = _weights_8;
    _weights[9] = _weights_9;
    _weights[10] = _weights_10;
    _weights[11] = _weights_11;
}
