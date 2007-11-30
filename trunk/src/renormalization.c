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

  (4)------------(5)------------(6)
  |               |              |
  |     (0)       |      (1)     | 
  |               |              |
  (7)---------------------------(8)
  |               |              |
  |     (2)       |     (3)      | 
  |               |              |
  (9)------------(10)------------(11)

  The weights of the possible edges are 0 if there is no edge, 
  and non zero if there is one
*/
static double _weights[NORMAL_NODES][NORMAL_NODES];

static Route _shortest_routes[BORDER_NODES][BORDER_NODES][BIT_CELL_MAX];

/*
 * Function which solves the Symmetric TSP by using renormalization technique
 */
void renormalize(Tsp *tsp)
{
    int cells_x, cells_y;
    int i, unity;
    
    int start, end;
    int old_x, old_y;
    int location;
    int cells_v;
    
    double min_x, min_y;
    double max_x, max_y;
    double range_x, range_y;
    
    int ind_x, ind_y;
    int **cells = {{NULL}};
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
     * Renormalize space until unity is reached. The renormalization procedure 
     * decreases the scale of the grid used for approximating the shortest route.
     * The space is divided into cells, the smaller the scale, the more cells.
     * For each cell there is checked if a city is within the cell. Now for each
     * block consisting of four cells the route is filled in. This route is 
     * optimal and should already be calculated by preprocess_routes. 
     * The previous iteration decides the entry and departure place of the block.
     */
    unity = 0;
    while (!unity) {
        /* Generate Cartesian grid, should also be done by a help function 
         *(here for testing purposes)
         */
        if ((cells= calloc(cells_x, sizeof(int*))) == NULL) 
            errx(EX_OSERR, "Out of memory");
        
        for (i = 0; i < cells_x; i++)
            if ((cells[i] = calloc(cells_y, sizeof(int))) == NULL) 
                errx(EX_OSERR, "Out of memory");
        

        for (ind_x = 0; ind_x < cells_x; ind_x++)
            for (ind_y = 0; ind_y < cells_y; ind_y++)
                cells[ind_x][ind_y] = 0;

        range_x = (max_x - min_x) / cells_x;
        range_y = (max_y - min_y) / cells_y;
        
        /* Check if a city is within a cell, should also be done 
         * by a help function (here for testing purposes)
         */
        unity = 1;
        for (i = 0; i < tsp->dimension; i++) {
            ind_x = (int)floor((tsp->cities[i].x - min_x) / range_x);
            ind_y = (int)floor((tsp->cities[i].y - min_y) / range_y);
            
            if (cells[ind_x][ind_y])
                unity = 0;
                
            cells[ind_x][ind_y] = 1;
        }
        
        /* Reserving space for the new route through the representative points*/
        if ((route_new = calloc(cells_x / 2, sizeof(Route**))) == NULL)
            errx(EX_OSERR, "Out of memory");
        
        /* For each block of four cells decide the shortest route which 
         * visits the centers of the cell where at least one
         * city is located
         */
        for (ind_x = 0; ind_x < cells_x  / 2; ind_x++)
            if ((route_new[ind_x] = calloc(cells_y / 2, sizeof(Route*))) == NULL)
                errx(EX_OSERR, "Out of memory");
        
        /* It is the first iteration, so entry and deperature 
         * points in a block are not an issue yet
         */
        for(ind_x = 0; ind_x < cells_x / 2; ind_x++) {
            for(ind_y = 0; ind_y < cells_y / 2; ind_y++) {
                cells_v = bitmask(cells[ind_x * 2][ind_y * 2],
                                cells[ind_x * 2][ind_y * 2 + 1],
                                cells[ind_x * 2 + 1][ind_y * 2],
                                cells[ind_x * 2 + 1][ind_y * 2 + 1]);
                if(!route_prev)
                    route_new[ind_x][ind_y] = get_basic_route(cells_v);
                else {
                    /* Get the entry and departure points in our 
                     * new route based on the previous route
                     */
                    old_x = (int)floor(ind_x / 2.0);
                    old_y = (int)floor(ind_y / 2.0);
                    //Get location and entry in array
                    if(route_prev[old_x][old_y]) {
                        start = route_prev[old_x][old_y]->start[ind_x % 2];
                        end = route_prev[old_x][old_y]->end[ind_x % 2];
                    }
                    else {
                        start = -1;
                        end = -1;
                    }
                    
                    if(start > 0 && end > 0)
                        route_new = &_shortest_routes[start][end][cells_v];
                    else
                        route_new = NULL;
                }
            }
        }
        
        /* For debugging ! */
        for (i = 0; i < cells_x; i++)
            free(cells[i]);
        free(cells);
        break;
        /* End debugging code */
    }
}

/*
 * Convert visited cells into a bitmask
 */
int bitmask(int cell_topleft, int cell_topright,
            int cell_bottomleft, int cell_bottomright)
{
    int mask = 0;
    
    if (cell_topleft)
        mask |= BIT_CELL_TL;
    if (cell_topright)
        mask |= BIT_CELL_TR;
    if (cell_bottomleft)
        mask |= BIT_CELL_BL;
    if (cell_bottomright)
        mask |= BIT_CELL_BR;
    
    return mask;
}
    
/*
 * Get the basic route. A basic route is a case where no entry point and 
 * departure point are specified on the edge of the square. For each cell
 * is specified if it needs to be visited or not using the arguments. 
 * The basic route is a closed path connecting the selected points
 */
Route* get_basic_route(int cells)
{
    Route* route;
    int previous_point = 0;
    
    if ((route = calloc(1, sizeof(Route))) == NULL)
        errx(EX_OSERR, "Out of memory");
    
    /* Simply visit all cells and you have already the shortest path */
    if (cells & BIT_CELL_TL) {
        route->trace[route->trace_length] = NODE_CELL_TL;
        route->trace_length++;
        
        previous_point = NODE_CELL_TL;
    }
    if (cells & BIT_CELL_TR) {
        if(previous_point) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_TR);
            route->trace_length++;
        }
        
        route->trace[route->trace_length] = NODE_CELL_TR;
        route->trace_length++;
        
        previous_point = NODE_CELL_TR;
    }
    if (cells & BIT_CELL_BR) {
        if(previous_point) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_BR);
            route->trace_length++;
        }
        
        route->trace[route->trace_length] = NODE_CELL_BR;
        route->trace_length++;
        
        previous_point = NODE_CELL_BR;
    }
    if (cells & BIT_CELL_BL) {
        if(previous_point) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_BL);
            route->trace_length++;
        }
        route->trace[route->trace_length] = NODE_CELL_BL;
        route->trace_length++;
    }
    
    /*Repeat last node to close the path (If the path is longer than two)*/
    if (route->trace_length > 2) {
        route->trace[route->trace_length] = 
                point_on_edge(route->trace[route->trace_length - 1], route->trace[0]);
        route->trace_length++;
        
        route->trace[route->trace_length] = route->trace[0];
        route->trace_length++;
    }
    
    return route;
}

/*
 * Initialize weight matrix of the default graph
 * Function is needed because this can not be done statically
 */
void make_weight_matrix()
{
    int i, j;
    //Initialize default on zero
    for(i = 0; i < NORMAL_NODES; i++)
        for(j = 0; j < NORMAL_NODES; j++)
            _weights[i][j] = 0.0;
     
    _weights[NODE_BORDER_TL][NODE_BORDER_T] = 1.0;
    _weights[NODE_BORDER_TL][NODE_BORDER_L] = 1.0;
    _weights[NODE_BORDER_TL][NODE_CELL_TL] = 0.707;
    
    _weights[NODE_BORDER_T][NODE_BORDER_TL] = 1.0;
    _weights[NODE_BORDER_T][NODE_BORDER_TR] = 1.0;
    _weights[NODE_BORDER_T][NODE_BORDER_B] = 2.0;
    _weights[NODE_BORDER_T][NODE_CELL_TL] = 0.707;
    _weights[NODE_BORDER_T][NODE_CELL_TR] = 0.707;
    
    _weights[NODE_BORDER_TR][NODE_BORDER_T] = 1.0;
    _weights[NODE_BORDER_TR][NODE_BORDER_R] = 1.0;
    _weights[NODE_BORDER_TR][NODE_CELL_TR] = 0.707;
    
    _weights[NODE_BORDER_L][NODE_BORDER_TL] = 1.0;
    _weights[NODE_BORDER_L][NODE_BORDER_BL] = 1.0;
    _weights[NODE_BORDER_L][NODE_BORDER_R] = 2.0;
    _weights[NODE_BORDER_L][NODE_CELL_TL] = 0.707;
    _weights[NODE_BORDER_L][NODE_CELL_BL] = 0.707;
    
    _weights[NODE_BORDER_R][NODE_BORDER_TR] = 1.0;
    _weights[NODE_BORDER_R][NODE_BORDER_BR] = 1.0;
    _weights[NODE_BORDER_R][NODE_BORDER_L] = 2.0;
    _weights[NODE_BORDER_R][NODE_CELL_TR] = 0.707;
    _weights[NODE_BORDER_R][NODE_CELL_BR] = 0.707;
    
    _weights[NODE_BORDER_BL][NODE_BORDER_L] = 1.0;
    _weights[NODE_BORDER_BL][NODE_BORDER_B] = 1.0;
    _weights[NODE_BORDER_BL][NODE_CELL_BL] = 0.707;
    
    _weights[NODE_BORDER_B][NODE_BORDER_BL] = 1.0;
    _weights[NODE_BORDER_B][NODE_BORDER_BR] = 1.0;
    _weights[NODE_BORDER_B][NODE_BORDER_T] = 2.0;
    _weights[NODE_BORDER_B][NODE_CELL_BR] = 0.707;
    _weights[NODE_BORDER_B][NODE_CELL_BL] = 0.707;
    
    _weights[NODE_BORDER_BR][NODE_BORDER_R] = 1.0;
    _weights[NODE_BORDER_BR][NODE_BORDER_B] = 1.0;
    _weights[NODE_BORDER_BR][NODE_CELL_BR] = 0.707;
    
    _weights[NODE_CELL_TL][NODE_CELL_TR] = 1.0;
    _weights[NODE_CELL_TL][NODE_CELL_BL] = 1.0;
    _weights[NODE_CELL_TL][NODE_CELL_BR] = 1.414;
    _weights[NODE_CELL_TL][NODE_BORDER_TL] = 0.707;
    _weights[NODE_CELL_TL][NODE_BORDER_T] = 0.707;
    _weights[NODE_CELL_TL][NODE_BORDER_L] = 0.707;
    
    _weights[NODE_CELL_TR][NODE_CELL_TL] = 1.0;
    _weights[NODE_CELL_TR][NODE_CELL_BR] = 1.0;
    _weights[NODE_CELL_TR][NODE_CELL_BL] = 1.414;
    _weights[NODE_CELL_TR][NODE_BORDER_TR] = 0.707;
    _weights[NODE_CELL_TR][NODE_BORDER_T] = 0.707;
    _weights[NODE_CELL_TR][NODE_BORDER_R] = 0.707;
    
    _weights[NODE_CELL_BL][NODE_CELL_BR] = 1.0;
    _weights[NODE_CELL_BL][NODE_CELL_TL] = 1.0;
    _weights[NODE_CELL_BL][NODE_CELL_TR] = 1.414;
    _weights[NODE_CELL_BL][NODE_BORDER_BL] = 0.707;
    _weights[NODE_CELL_BL][NODE_BORDER_B] = 0.707;
    _weights[NODE_CELL_BL][NODE_BORDER_L] = 0.707;
    
    _weights[NODE_CELL_BR][NODE_CELL_BL] = 1.0;
    _weights[NODE_CELL_BR][NODE_CELL_TR] = 1.0;
    _weights[NODE_CELL_BR][NODE_CELL_TL] = 1.414;
    _weights[NODE_CELL_BR][NODE_BORDER_BR] = 0.707;
    _weights[NODE_CELL_BR][NODE_BORDER_B] = 0.707;
    _weights[NODE_CELL_BR][NODE_BORDER_R] = 0.707;
}

/*
 * Calculate all shortest routes for the default graph(See top of this file). 
 * Here difference is made for the cells which are visited. For all 
 * the nodes shortest paths are calculated for every possible combination of 
 * cells which need to be visited. The visited cells are encoded using bitmasks,
 * for making the iteration more simple.
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
    for (start = 0; start < BORDER_NODES; start++) {
        for (end = 0; end < BORDER_NODES; end++) {
            path = paths(start + CELL_NODES, end + CELL_NODES, 0);

            /*Decide for each possible combination of visited cells, what the shortest route is*/
            for (cells = 0; cells < BIT_CELL_MAX; cells++) {
                shortest = NULL;
                for (i = 0; i < path.size; i++) {
                    if (route_visits_cells(path.routes[i], cells)) {
                        if (shortest == NULL)
                            shortest = path.routes[i];
                        else if (path.routes[i]->length < shortest->length)
                            shortest = path.routes[i];
                    }
                }
                printf("From %d to %d, via %d\n", start + CELL_NODES, end + CELL_NODES, cells);
                assert(shortest);
                copy_route(&_shortest_routes[start][end][cells], shortest);

            }
            free(path.routes);
        }
    }
}

/*
 * Finds entry and departure blocks in one route block 
 */
 void set_borderpoints_subblocks(Route* route)
{
    int i;
    int start_cur, end_cur;
    int cell_a, cell_b;
    int has_start = 0;
    
    if(!route)
        return;
    
    for(i = 0; i < route->trace_length; i++) {
        if(route->trace[i] >= NODE_CELL_TL && route->trace[i] <= NODE_CELL_BR)
            continue;
        
        if(!has_start) {
            start_cur = route->trace[i];
            has_start = 1;
        }
        else {
            end_cur = route->trace[i];
            get_cell_index(start_cur, end_cur, &cell_a, &cell_b);
            route->start[cell_a] = start_cur;
            route->end[cell_a] = end_cur;
        }
    }
}
/*
 * Function giving all possible paths between start and end, which visit each 
 * node maximal one time. It results a list of pointers to the found routes
 */
Route_array paths(int start, int end, int visited)
{
    Route_array routes_new;
    Route_array routes_cur;
    Route* route;
    int paths_previous_no, i, j, k;
    int node_between;
    int diff;
    
    routes_new.routes = NULL;
    routes_new.size = 0;

    /* Function wants all the routes with the same start and endpoint for 
     * calculating the shortest path, Normally you would aspect that this path 
     * has a length of 0 (Stay at the location). However maybe he needs to
     * visit some cells
     */
    if (start == end && visited == 0) {
        /* Find edges out of the start point. Use the destination of these edge 
         * as a start point for a route to our endpoint, refuse routes 
         * which use the same edge we have selected, since it is not 
         * allowed to use an edge twice
         */
        for (i = 0; i < NORMAL_NODES; i++) {
            if (_weights[i][end]) {
                routes_cur = paths(i, end, 0);
                node_between = point_on_edge(i, end);
        
                for (j = 0; j < routes_cur.size; j++) {
                    route = routes_cur.routes[j];
                    
                    /* Route is one edge long uses this discovered edge */
                    if (route->trace_length == 2) {
                        free(route);
                        continue;
                    }
                    else if (route->trace_length == 3 && 
                              route->trace[0] == start &&
                              route->trace[1] == node_between &&
                              route->trace[2] == i) {
                        free(route);
                        continue;
                    }
                    
                    diff = node_between == -1 ? 1 : 2;
                    
                    /* Put our startpoint in front again to make round tour */
                    for (k = route->trace_length; k > diff - 1; k--)
                        route->trace[k] = route->trace[k - diff];
                    
                    route->trace[0] = start;
                    route->trace_length++;
                    route->length += _weights[start][i]; 


                    if (node_between != -1) {
                        route->trace[1] = node_between;
                        route->trace_length++;
                    }
                    
                    add_route(&routes_new, route);
                }
                free(routes_cur.routes);
            }
        }
    }
    
    /* Base case: all paths from a point to itself. 
     * Since we are eventually interested in shortest paths,
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
    
    /* For getting all possible paths you search all connected neighbours. 
     * From here the paths are recursively determined. Now you only need to
     * add this current point to the trace. Do not visit neighbours which 
     * are already visited in the current route, since this would produce 
     * cycles in the route
     */
    else {
        for (i = 0; i < NORMAL_NODES; i++) {
            if (_weights[i][end] && !(visited & (1 << i))) {
                routes_cur = paths(start, i, visited | (1 << end));
                node_between = point_on_edge(i, end);
        
                for (j = 0; j < routes_cur.size; j++) {
                    route = routes_cur.routes[j];
                    
                    /* Edge this edge to the route (With the node in between) */
                    if (node_between != -1) {
                        route->trace[route->trace_length] = node_between;
                        route->trace_length++;
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
    return routes_new;
}

/*
 * Add a route to a route array. 
 * Herefore the allocated size of the array is increased
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
 *  Copy a route
 */
void copy_route(Route* dest, Route* src)
{
    int i;
    
    printf("A shortest route[%f]!: ", src->length);
    for (i = 0; i < NODES; i++) {
        dest->trace[i] = src->trace[i]; printf("%d.", src->trace[i]); }
        printf("\n");
        dest->trace_length = src->trace_length;
        dest->length = src->length;
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
 * Check if the route is valid and visits the cells specified in 
 * the bitmask <cells>
 */
int route_visits_cells(Route* route, int cells)
{
    if (BIT_CELL_TL & cells && !node_in_route(NODE_CELL_TL, route))
        return 0;

    if (BIT_CELL_TR & cells && !node_in_route(NODE_CELL_TR, route))
        return 0;

    if (BIT_CELL_BL & cells && !node_in_route(NODE_CELL_BL, route))
        return 0;

    if (BIT_CELL_BR & cells && !node_in_route(NODE_CELL_BR, route))
        return 0;
    
    return 1;
}

/* 
 * Check if there are extra visitable points on the edge. These points
 * are as follows:
 * incorrect picture!!!!
 * (0)------------(1)------------(2)
 * |               |              |
 * |     (8)      [12]    (9)     | 
 * |               |              |
 * (3)---[13]-----[14]---[15]----(4)
 * |               |              |
 * |     (10)     [16]   (11)     | 
 * |               |              |
 * (5)------------(6)------------(7)
 *
 * if so return the number of the edge(12-16). Otherwise return -1;
 */
int point_on_edge(int edge_start, int edge_finish)
{
    int swap;
    
    if (edge_start > edge_finish) {
        swap = edge_start;
        edge_start = edge_finish;
        edge_finish = swap;
    }
    
    if(edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_TR)
        return NODE_CROSS_T;
    if(edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_BL)
        return NODE_CROSS_L;
    if(edge_start == NODE_BORDER_T && edge_finish == NODE_BORDER_B)
        return NODE_CROSS_C;
    if(edge_start == NODE_BORDER_L && edge_finish == NODE_BORDER_R)
        return NODE_CROSS_C;
    if(edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_BR)
        return NODE_CROSS_C;
    if(edge_start == NODE_CELL_TR && edge_finish == NODE_CELL_BL)
        return NODE_CROSS_C;
    if(edge_start == NODE_CELL_TR && edge_finish == NODE_CELL_BR)
        return NODE_CROSS_R;
    if(edge_start == NODE_CELL_BL && edge_finish == NODE_CELL_BR)
        return NODE_CROSS_B;
    
    return -1;
}

void get_cell_index(int start, int end, int *cell_a, int *cell_b)
{
    int start_in_cell[4];
    int end_in_cell[4];
    int i;
    
    for(i = 0; i < 4; i++) {
        start_in_cell[i] = 0;
        end_in_cell[i] = 0;
    }
    
    *cell_a = -1;
    *cell_b = -1;
    
    
    if(start == NODE_BORDER_TL || start == NODE_BORDER_T ||
       start == NODE_BORDER_L || start == NODE_CROSS_T ||
       start == NODE_CROSS_L || start == NODE_CROSS_C)
        start_in_cell[NODE_CELL_TL] = 1;
    
    if(start == NODE_BORDER_TR || start == NODE_BORDER_T ||
       start == NODE_BORDER_R || start == NODE_CROSS_T ||
       start == NODE_CROSS_R || start == NODE_CROSS_C)
        start_in_cell[NODE_CELL_TR] = 1;
    
    if(start == NODE_BORDER_BL || start == NODE_BORDER_B ||
       start == NODE_BORDER_L || start == NODE_CROSS_B ||
       start == NODE_CROSS_L || start == NODE_CROSS_C)
        start_in_cell[NODE_CELL_BL] = 1;
    
    if(start == NODE_BORDER_BR || start == NODE_BORDER_B ||
       start == NODE_BORDER_R || start == NODE_CROSS_B ||
       start == NODE_CROSS_R || start == NODE_CROSS_C)
        start_in_cell[NODE_CELL_BR] = 1;
    
    if(end == NODE_BORDER_TL || end == NODE_BORDER_T ||
       end == NODE_BORDER_L || end == NODE_CROSS_T ||
       end == NODE_CROSS_L || end == NODE_CROSS_C)
        end_in_cell[NODE_CELL_TL] = 1;
    
    if(end == NODE_BORDER_TR || end == NODE_BORDER_T ||
       end == NODE_BORDER_R || end == NODE_CROSS_T ||
       end == NODE_CROSS_R || end == NODE_CROSS_C)
        end_in_cell[NODE_CELL_TR] = 1;
    
    if(end == NODE_BORDER_BL || end == NODE_BORDER_B ||
       end == NODE_BORDER_L || end == NODE_CROSS_B ||
       end == NODE_CROSS_L || end == NODE_CROSS_C)
        end_in_cell[NODE_CELL_BL] = 1;
    
    if(end == NODE_BORDER_BR || end == NODE_BORDER_B ||
       end == NODE_BORDER_R || end == NODE_CROSS_B ||
       end == NODE_CROSS_R || end == NODE_CROSS_C)
        end_in_cell[NODE_CELL_BR] = 1;
    
    for(i = 0; i < 4; i++) {
        if(start_in_cell[i] && end_in_cell[i]) {
            if(*cell_a)
                *cell_b = i;
            else
                *cell_a = i;
        }
    }
}
