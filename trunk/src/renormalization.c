#include <assert.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>

#include "block.h"
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

Route* _shortest_routes[BORDER_NODES][BORDER_NODES][BIT_CELL_MAX];

/*
 * Function which solves the Symmetric TSP by using renormalization technique
 */
int* renormalize()
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
    grd* grid;
    Route ***route_new = NULL;
    Route ***route_prev = NULL;
    
    /* Set up grid, only for testing purposes, should call help function */
    cells_x = 2;
    cells_y = 2;
    
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
        printf("Iteration %d\n", (int)log2(cells_x));
        /* Generate Cartesian grid, should also be done by a help function 
         *(here for testing purposes)
         */
        grid = create_grd(&cells_x, &cells_y);
        
        /* Check if we are already at unity */
        unity = 1;
        for (ind_x = 0; ind_x < cells_x; ind_x++)
            for (ind_y = 0; ind_y < cells_y; ind_y++)
                if (has_cities(grid->block[ind_x][ind_y]))
                    unity = 0;
        
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
                cells_v = bitmask(grid, ind_x * 2, ind_y * 2);
                if(!route_prev)
                    route_new[ind_x][ind_y] = get_basic_route(cells_v);
                else {
                    /* Get the entry and departure points in our 
                     * new route based on the previous route
                     */
                    old_x = floor(ind_x / 2.0);
                    old_y = floor(ind_y / 2.0);
                    
                    location = (ind_x % 2) + 2 * (ind_y % 2);
                    
                    //Get location and entry in array
                    if(route_prev[old_x][old_y]) {
                        start = route_prev[old_x][old_y]->start[location];
                        end = route_prev[old_x][old_y]->end[location];
                    }
                    else {
                        start = -1;
                        end = -1;
                    }
                    
                    if(start != -1 && end != -1) {
                        route_new[ind_x][ind_y] = 
                          _shortest_routes[start - CELL_NODES][end - CELL_NODES][cells_v];
                    }
                    else
                        route_new[ind_x][ind_y] = NULL;
                }
            }
        }
        route_prev = route_new;
        
        if (unity)
            break;
        
        free_grd(grid);
        grid = NULL;
        
        cells_x *= 2;
        cells_y *= 2;
        /* End debugging code */
    }
    
    int* result = map_on_route(route_prev, grid, cells_x, cells_y);
    free_grd(grid);
    
    return result;
}

int* map_on_route(Route*** routeblock, grd *grid, int cells_x, int cells_y)
{
    int* route;
    int visited, i;
    int ind_x, ind_y;
    Route *block_cur, *block_start;
    
    visited = 0;
    if((route = calloc(tsp->dimension, sizeof(int))) == NULL)
        errx(EX_OSERR, "Out of memory");
    
    //Find first spot to retrace route
    for (ind_x = 0; ind_x < cells_x / 2; ind_x++)
        for (ind_y = 0; ind_y < cells_y / 2; ind_y++)
            if (routeblock[ind_x][ind_y])
                goto found;
    errx(1, "Nothing found!");
found:
    block_start = routeblock[ind_x][ind_y];
    block_cur = block_start;
    
    while(1) {
        for (i = 0; i < block_cur->trace_length; i++) {
            switch (block_cur->trace[i]) {
                case NODE_CELL_TL:
                    if(has_city(grid->block[2 * ind_x][2 * ind_y])) {
                        route[visited] = grid->block[2 * ind_x][2 * ind_y];
                        visited++;                       
                    }
                    break;
                case NODE_CELL_TR:
                    if(has_city(grid->block[2 * ind_x + 1][2 * ind_y])) {
                        route[visited] = grid->block[2 * ind_x + 1][2 * ind_y];
                        visited++;                       
                    }
                    break;
                case NODE_CELL_BL:
                    if(has_city(grid->block[2 * ind_x][2 * ind_y + 1])) {
                        route[visited] = grid->block[2 * ind_x][2 * ind_y + 1];
                        visited++;                       
                    }
                    break;
                case NODE_CELL_BR:
                    if(has_city(grid->block[2 * ind_x + 1][2 * ind_y + 1])) {
                        route[visited] = grid->block[2 * ind_x + 1][2 * ind_y + 1];
                        visited++;                       
                    }
                    break;
            }
        }
        get_next_cell(routeblock, &ind_x, &ind_y, cells_x, cells_y);
        block_cur = routeblock[ind_x][ind_y];

        if (block_cur == block_start)
            break;
    }    
    return route;
}
/*
 * Convert visited cells into a bitmask
 */
int bitmask(grd *grid, int ind_x, int ind_y)
{
    int mask = 0;
    
    if (has_city(grid->block[ind_x][ind_y]))
        mask |= BIT_CELL_TL;
    if (has_city(grid->block[ind_x + 1][ind_y]))
        mask |= BIT_CELL_TR;
    if (has_city(grid->block[ind_x][ind_y + 1]))
        mask |= BIT_CELL_BL;
    if (has_city(grid->block[ind_x + 1][ind_y + 1]))
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
    int i;
    int previous_point = -1;
    
    if ((route = calloc(1, sizeof(Route))) == NULL)
        errx(EX_OSERR, "Out of memory");

    /* Simply visit all cells and you have already the shortest path */
    if (cells & BIT_CELL_TL) {
        route->trace[route->trace_length] = NODE_CELL_TL;
        route->trace_length++;
        
        previous_point = NODE_CELL_TL;
    }
    if (cells & BIT_CELL_TR) {
        if(previous_point != -1) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_TR);
            route->trace_length++;
        }
        
        route->trace[route->trace_length] = NODE_CELL_TR;
        route->trace_length++;
        
        previous_point = NODE_CELL_TR;
    }
    if (cells & BIT_CELL_BR) {
        if(previous_point != -1) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_BR);
            route->trace_length++;
        }
        
        route->trace[route->trace_length] = NODE_CELL_BR;
        route->trace_length++;
        
        previous_point = NODE_CELL_BR;
    }
    if (cells & BIT_CELL_BL) {
        if(previous_point != -1) {
            route->trace[route->trace_length] = 
                    point_on_edge(previous_point, NODE_CELL_BL);
            route->trace_length++;
        }
        route->trace[route->trace_length] = NODE_CELL_BL;
        route->trace_length++;
    }
    
    /*Repeat last node to close the path (If the path is longer than two) 
      and set start and endpoints in subblock*/
    if (route->trace_length > 3) {
        previous_point = 
           point_on_edge(route->trace[route->trace_length - 1], route->trace[0]);
        
        for (i = route->trace_length; i > 0; i--)
            route->trace[i] = route->trace[i - 1];
        route->trace_length++;

        route->trace[0] = previous_point;
        route->trace[route->trace_length] = previous_point;
        route->trace_length++;
        
        set_borderpoints_subblocks(route);
    }
    else {
        errx(EX_DATAERR, "Try other range!\n");
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
     
//    _weights[NODE_BORDER_TL][NODE_BORDER_T] = 1.0;
//    _weights[NODE_BORDER_TL][NODE_BORDER_L] = 1.0;
    _weights[NODE_BORDER_TL][NODE_CELL_TL] = 0.707;
    
//    _weights[NODE_BORDER_T][NODE_BORDER_TL] = 1.0;
//    _weights[NODE_BORDER_T][NODE_BORDER_TR] = 1.0;
//    _weights[NODE_BORDER_T][NODE_BORDER_B] = 2.0;
    _weights[NODE_BORDER_T][NODE_CELL_TL] = 0.707;
    _weights[NODE_BORDER_T][NODE_CELL_TR] = 0.707;
    
//    _weights[NODE_BORDER_TR][NODE_BORDER_T] = 1.0;
//    _weights[NODE_BORDER_TR][NODE_BORDER_R] = 1.0;
    _weights[NODE_BORDER_TR][NODE_CELL_TR] = 0.707;
    
//    _weights[NODE_BORDER_L][NODE_BORDER_TL] = 1.0;
//    _weights[NODE_BORDER_L][NODE_BORDER_BL] = 1.0;
//    _weights[NODE_BORDER_L][NODE_BORDER_R] = 2.0;
    _weights[NODE_BORDER_L][NODE_CELL_TL] = 0.707;
    _weights[NODE_BORDER_L][NODE_CELL_BL] = 0.707;
    
//    _weights[NODE_BORDER_R][NODE_BORDER_TR] = 1.0;
//    _weights[NODE_BORDER_R][NODE_BORDER_BR] = 1.0;
//    _weights[NODE_BORDER_R][NODE_BORDER_L] = 2.0;
    _weights[NODE_BORDER_R][NODE_CELL_TR] = 0.707;
    _weights[NODE_BORDER_R][NODE_CELL_BR] = 0.707;
    
//    _weights[NODE_BORDER_BL][NODE_BORDER_L] = 1.0;
//    _weights[NODE_BORDER_BL][NODE_BORDER_B] = 1.0;
    _weights[NODE_BORDER_BL][NODE_CELL_BL] = 0.707;
    
//    _weights[NODE_BORDER_B][NODE_BORDER_BL] = 1.0;
//    _weights[NODE_BORDER_B][NODE_BORDER_BR] = 1.0;
//    _weights[NODE_BORDER_B][NODE_BORDER_T] = 2.0;
    _weights[NODE_BORDER_B][NODE_CELL_BR] = 0.707;
    _weights[NODE_BORDER_B][NODE_CELL_BL] = 0.707;
    
//    _weights[NODE_BORDER_BR][NODE_BORDER_R] = 1.0;
//    _weights[NODE_BORDER_BR][NODE_BORDER_B] = 1.0;
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
                if (!shortest)
                    _shortest_routes[start][end][cells] = NULL;
                else {
                    _shortest_routes[start][end][cells] = copy_route(shortest);
                    set_borderpoints_subblocks(_shortest_routes[start][end][cells]);
                }
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
    int alternative[CELL_NODES];
    
    if(!route)
        return;
    for (i = 0; i < CELL_NODES; i++) {
        route->start[i] = -1;
        route->end[i] = -1;        
    }
    for(i = 0; i < route->trace_length; i++) {
        if(route->trace[i] >= NODE_CELL_TL && route->trace[i] <= NODE_CELL_BR)
            continue;
        
        if(!has_start) {
            start_cur = i;
            has_start = 1;
        }
        else {
            end_cur = i;
            get_cell_index(route, start_cur, end_cur, &cell_a, &cell_b);
            
            if(route->start[cell_a] == -1) {
                route->start[cell_a] = route->trace[start_cur];
                route->end[cell_a] = route->trace[end_cur];

                alternative[cell_a] = cell_b;
            }
            else if(route->start[cell_b] == -1) {
                route->start[cell_b] = route->trace[start_cur];
                route->end[cell_b] = route->trace[end_cur];
                
                alternative[cell_b] = cell_b;
            }
            else if(alternative[cell_a]){
                route->start[alternative[cell_a]] = route->start[cell_a];
                route->end[alternative[cell_a]] = route->end[cell_a];

                route->start[cell_a] = route->trace[start_cur];
                route->end[cell_a] = route->trace[end_cur];
            }
            else if(alternative[cell_b]){
                route->start[alternative[cell_b]] = route->start[cell_b];
                route->end[alternative[cell_b]] = route->end[cell_b];
                
                route->start[cell_b] = route->trace[start_cur];
                route->end[cell_b] = route->trace[end_cur];
            }
                        
            start_cur = end_cur;
        }
    }
    
    for (i = 0; i < CELL_NODES; i++) {
        route->start[i] = convert_node(i, route->start[i]);
        route->end[i] = convert_node(i, route->end[i]);
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
Route* copy_route(Route* src)
{
    int i;
    Route* dest;
    
    if ((dest = calloc(1, sizeof(Route))) == NULL)
        errx(EX_OSERR, "Out of memory");

    for (i = 0; i < NODES; i++)
        dest->trace[i] = src->trace[i];

        dest->trace_length = src->trace_length;
        dest->length = src->length;
        
        for (i = 0; i < CELL_NODES; i++) {
            dest->start[i] = src->start[i];
            dest->end[i] = src->end[i];
        }
            
        return dest;
}

/*
 * Check if a specific node is in the route
 */
int node_in_route(int node, Route *route)
{
    int i;
    
    if (!route)
        return 0;

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

void get_corresponding_cell(int point, int *cell_a, int *cell_b)
{
    *cell_a = -1;
    *cell_b = -1;
    
    switch(point) {
        case NODE_BORDER_TL:
            *cell_a = NODE_CELL_TL;
            break;
        case NODE_BORDER_T:
            *cell_a = NODE_CELL_TL;
            *cell_b = NODE_CELL_TR;
            break;
        case NODE_BORDER_TR:
            *cell_a = NODE_CELL_TR;
            break;
        case NODE_BORDER_L:
            *cell_a = NODE_CELL_TL;
            *cell_b = NODE_CELL_BL;
            break;
        case NODE_BORDER_R:
            *cell_a = NODE_CELL_TR;
            *cell_b = NODE_CELL_BR;
            break;
        case NODE_BORDER_BL:
            *cell_a = NODE_CELL_BL;
            break;
        case NODE_BORDER_B:
            *cell_a = NODE_CELL_BL;
            *cell_b = NODE_CELL_BR;
            break;
        case NODE_BORDER_BR:
            *cell_a = NODE_CELL_BR;
            break;
        case NODE_CROSS_T:
            *cell_a = NODE_CELL_TL;
            *cell_b = NODE_CELL_TR;
            break;
        case NODE_CROSS_L:
            *cell_a = NODE_CELL_TL;
            *cell_b = NODE_CELL_BL;
            break;
        case NODE_CROSS_C:
            *cell_a = ANY_NODE_CELL;
            break;
        case NODE_CROSS_R:
            *cell_a = NODE_CELL_TR;
            *cell_b = NODE_CELL_BR;
            break;
        case NODE_CROSS_B:
            *cell_a = NODE_CELL_BL;
            *cell_b = NODE_CELL_BR;
            break;
    }
}

void get_cell_index(Route* route, int start, int end, int *cell_a, int *cell_b)
{
    int start_in_cell[CELL_NODES];
    int end_in_cell[CELL_NODES];
    int is_path;
    int i, j;
    
    for(i = 0; i < CELL_NODES; i++) {
        start_in_cell[i] = 0;
        end_in_cell[i] = 0;
    }
    
    *cell_a = -1;
    *cell_b = -1;
    
    get_corresponding_cell(route->trace[start], cell_a, cell_b);
    if (*cell_a == ANY_NODE_CELL)
        for(i = 0; i < CELL_NODES; i++)
            start_in_cell[i] = 1; 
    else if (*cell_a != -1)
        start_in_cell[*cell_a] = 1;
    if (*cell_b != -1)
        start_in_cell[*cell_b] = 1;
    
    get_corresponding_cell(route->trace[end], cell_a, cell_b);
    if (*cell_a == ANY_NODE_CELL)
        for(i = 0; i < CELL_NODES; i++)
            end_in_cell[i] = 1; 
    else if (*cell_a != -1)
        end_in_cell[*cell_a] = 1;
    if (*cell_b != -1)
        end_in_cell[*cell_b] = 1;

    *cell_a = -1;
    *cell_b = -1;

    for(i = 0; i < CELL_NODES; i++) {
        if(start_in_cell[i] && end_in_cell[i]) {
            is_path = 1;
            for (j = start + 1; j < end; j++)
                if (route->trace[j] != i)
                    is_path = 0;
            
            if(!is_path)
                continue;
            
            if(*cell_a != -1)
                *cell_b = i;
            else
                *cell_a = i;
        }
    }
}

void get_next_cell(Route*** route, int* x, int* y, int cells_x, int cells_y)
{
    int border_point;
    int cell_point, cell;
    int cell_a, cell_b;
    int i;
    
    border_point = route[*x][*y]->trace[route[*x][*y]->trace_length - 1];

    switch(border_point) {
        case NODE_BORDER_TL:
            if(*x > 0 && *y > 0 &&
               node_in_route(NODE_BORDER_BR, route[*x - 1][*y - 1])) {
                *x = *x - 1;
                *y = *y - 1;
            }
            else if(*x > 0 && 
                    node_in_route(NODE_BORDER_TR, route[*x - 1][*y])) {
                *x = *x - 1;
            }
            else {
                *y = *y - 1;
            }
            break;
        case NODE_BORDER_T:
            *y = *y - 1;
            break;
        case NODE_BORDER_TR:
            if(*x + 1 < cells_x / 2 && *y > 0 &&
               node_in_route(NODE_BORDER_BL, route[*x + 1][*y - 1])) {
                *x = *x + 1;
                *y = *y - 1;
            }
            else if(*x + 1 < cells_x / 2 && 
                    node_in_route(NODE_BORDER_TL, route[*x + 1][*y])) {
                *x = *x + 1;
            }
            else {
                *y = *y - 1;
            }
            break;
        case NODE_BORDER_L:
            *x = *x - 1;
            break;
        case NODE_BORDER_R:
            *x = *x + 1;
            break;
        case NODE_BORDER_BL:
            if(*x > 0 && *y + 1 < cells_y / 2 &&
               node_in_route(NODE_BORDER_TR, route[*x - 1][*y + 1])) {
                *x = *x - 1;
                *y = *y + 1;
            }
            else if(*x > 0 && 
                    node_in_route(NODE_BORDER_BR, route[*x - 1][*y])) {
                *x = *x - 1;
            }
            else {
                *y = *y + 1;
            }
            break;
        case NODE_BORDER_B:
            *y = *y + 1;
            break;
        case NODE_BORDER_BR:
            if(*x + 1 < cells_x / 2 && *y + 1 < cells_y / 2 &&
               node_in_route(NODE_BORDER_TL, route[*x + 1][*y + 1])) {
                *x = *x + 1;
                *y = *y + 1;
            }
            else if(*x + 1 < cells_x / 2 && 
                    node_in_route(NODE_BORDER_BL, route[*x + 1][*y])) {
                *x = *x + 1;
            }
            else {
                *y = *y + 1;
            }
            break;
    }
}

int convert_node(int location, int main_node)
{
    switch(location)
    {
        case NODE_CELL_TL:
            switch(main_node) {
                case NODE_BORDER_T:
                    return NODE_BORDER_TR;
                case NODE_BORDER_L:
                    return NODE_BORDER_BL;
                case NODE_CROSS_T:
                    return NODE_BORDER_R;
                case NODE_CROSS_L:
                    return NODE_BORDER_B;
                case NODE_CROSS_C:
                    return NODE_BORDER_BR;
            }
            break;
        case NODE_CELL_TR:
            switch(main_node) {
                case NODE_BORDER_T:
                    return NODE_BORDER_TL;
                case NODE_BORDER_R:
                    return NODE_BORDER_BR;
                case NODE_CROSS_T:
                    return NODE_BORDER_L;
                case NODE_CROSS_C:
                    return NODE_BORDER_BL;
                case NODE_CROSS_R:
                    return NODE_BORDER_B;
            }
            break;
        case NODE_CELL_BL:
            switch(main_node) {
                case NODE_BORDER_L:
                    return NODE_BORDER_TL;
                case NODE_BORDER_B:
                    return NODE_BORDER_BR;
                case NODE_CROSS_L:
                    return NODE_BORDER_T;
                case NODE_CROSS_C:
                    return NODE_BORDER_TR;
                case NODE_CROSS_B:
                    return NODE_BORDER_R;
            }
            break;
        case NODE_CELL_BR:
            switch(main_node) {
                case NODE_BORDER_R:
                    return NODE_BORDER_TR;
                case NODE_BORDER_B:
                    return NODE_BORDER_BL;
                case NODE_CROSS_C:
                    return NODE_BORDER_TL;
                case NODE_CROSS_R:
                    return NODE_BORDER_T;
                case NODE_CROSS_B:
                    return NODE_BORDER_L;
            }
            break;
    }
    return main_node;
}
