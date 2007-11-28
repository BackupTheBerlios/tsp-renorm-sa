#include <assert.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>

#include "tsp.h"
#include "renormalization.h"
#include "io.h"
//Static precede with _
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
static double weights_0[] = {0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.0, 0.0};
static double weights_1[] = {1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.707, 0.707, 0.0, 0.0};
static double weights_2[] = {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.0};
static double weights_3[] = {1.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.0};
static double weights_4[] = {0.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.707, 0.0, 0.707};
static double weights_5[] = {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.707, 0.0};
static double weights_6[] = {0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.707, 0.707};
static double weights_7[] = {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.707};
static double weights_8[] = {0.707, 0.707, 0.0, 0.707, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.414};
static double weights_9[] = {0.0, 0.707, 0.707, 0.0, 0.707, 0.0, 0.0, 0.0, 1.0, 0.0, 1.414, 1.0};
static double weights_10[] = {0.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.707, 0.0, 1.0, 1.414, 0.0, 1.0};
static double weights_11[] = {0.0, 0.0, 0.0, 0.0, 0.707, 0.0, 0.707, 0.707, 1.141, 1.0, 1.0, 0.0};

static double *weights[12];

Route shortest_routes[9][9][16];

void renormalize(Tsp *tsp)
{
    int cells_x, cells_y;
    int i, x, y, unity;
    double min_x, min_y;
    double max_x, max_y;
    double range_x, range_y;
    int ind_x, ind_y;
    int **cells = NULL;
    Route **route_new = NULL;
    Route **route_prev = NULL;
    
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
    
    unity = 0;
    while (!unity) {
        //Generate Cartesian grid
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
        
        //Check if a city is within a cell
        unity = 1;
        for (i = 0; i < tsp->dimension; i++) {
            ind_x = (int)floor(tsp->cities[i].x - min_x / range_x);
            ind_y = (int)floor(tsp->cities[i].y - min_y / range_y);
            
            if (cells[ind_x][ind_y])
                unity = 0;
                
            cells[ind_x][ind_y] = 1;
        }
        
        //Generate a route between the visited cells
        if ((route_new = calloc(cells_x / 2, sizeof(Route*))) == NULL)
            errx(EX_OSERR, "Out of memory");
        
        for (x = 0; x < cells_x  / 2; x++) {
            for (y = 0; y < cells_y / 2; y++) {
                if (route_prev) {
                    //...
                }
                else {
                    if()
                    route_new[x][y] = getRoute(cells[x * 2][y * 2], 
                                               cells[x * 2][y * 2 + 1],
                                               cells[x * 2 + 1][y * 2],
                                               cells[x * 2 + 1][y * 2 + 1], 0, 0);
                }
            }
        }
    }
}

int getRoute(int cell_topleft, int cell_topright,
             int cell_bottomleft, int cell_bottomright,
             int route_part, int route_cur)
{
    int route = 0;
    
    if (cell_topleft)
        route |= CELL_TOPLEFT;
    if (cell_topright)
        route |= CELL_TOPRIGHT;
    if (cell_bottomleft)
        route |= CELL_BOTTOMLEFT;
    if (cell_bottomright)
        route |= CELL_BOTTOMRIGHT;

    switch(route_part) {
        case CELL_TOPLEFT:
            
            break;
        case CELL_TOPRIGHT:
            break;
        case CELL_BOTTOMLEFT:
            break;
        case CELL_BOTTOMRIGHT:
            break;
        default:
            break;
    }
    
    return route;
}

void preprocess_routes()
{
    int visits;
    int start, end;
    int i, j;
    int paths_possible;
    int visit_topleft, visit_topright;
    int visit_bottomleft, visit_bottomright;
    int is_shorter;
    Route_array path;
    Route* shortest;
    
    make_weight_matrix();
    
    for (start = 0; start < 9; start++) {
        for (end = 0; end < 9; end++) {
            path = paths(start, end, 0);

            for (visits = 0; visits < 16; visits++) {
                if (start != end) {
                    shortest = NULL;
                    
                    for (i = 0; i < path.size; i++) {
                        visit_topleft = !(CELL_TOPLEFT & visits) ||
                                (CELL_TOPLEFT & visits && 
                                node_in_route(NODE_TOPLEFT, path.routes[i]));
                        visit_topright = !(CELL_TOPRIGHT & visits) ||
                                (CELL_TOPRIGHT & visits && 
                                node_in_route(NODE_TOPRIGHT, path.routes[i]));
                        visit_bottomleft = !(CELL_BOTTOMLEFT & visits) ||
                                (CELL_BOTTOMLEFT & visits && 
                                node_in_route(NODE_BOTTOMLEFT, path.routes[i]));
                        visit_bottomright = !(CELL_BOTTOMRIGHT & visits) ||
                                (CELL_BOTTOMRIGHT & visits && 
                                node_in_route(NODE_BOTTOMRIGHT, path.routes[i]));
                        is_shorter = !shortest || 
                                (path.routes[i]->length < shortest->length);
                        if (visit_topleft && visit_topright && visit_bottomleft
                           && visit_bottomright && is_shorter)
                            shortest = path.routes[i];
                    }
                    assert(shortest);
                    for(i = 0; i < NODES; i++)
                        shortest_routes[start][end][visits].trace[i] =
                                shortest[i].trace[i];
                    
                    shortest_routes[start][end][visits].trace_length =
                            shortest->trace_length;
                    
                    shortest_routes[start][end][visits].length =
                            shortest->length;
                }
            }
            free(path.routes);
        }
    }
}
/*
  Function giving all possible paths between start and end, which visit each 
  node maximal one time. It results a list of pointers to the found routes
 */
Route_array paths(int start, int end, int visited)
{
    Route_array paths_result;
    Route_array paths_previous;
    Route* route_new;
    int paths_previous_no, i, j;
    
    paths_result.routes = NULL;
    paths_result.size = 0;
    
    if(start == end) {
        if ((route_new = calloc(1, sizeof(Route))) == NULL)
            errx(EX_OSERR, "Out of memory");
        
        route_new->trace[0] = start;
        route_new->trace_length = 1;
        route_new->length = 0.0;
        add_route(&paths_result, route_new);
        return paths_result;
    }
    else {
        for(i = 0; i < NODES; i++) {
            if(weights[i][end] && !(visited & (1 << i))) {
                paths_previous = paths(start, i, visited | (1 << end));
        
                for(j = 0; j < paths_previous.size; j++) {
                    
                    route_new = paths_previous.routes[j];
                    route_new->trace[route_new->trace_length] = end;
                    route_new->trace_length++;
                    route_new->length += weights[i][end]; 
            
                    add_route(&paths_result, route_new);
                }
                free(paths_previous.routes);
            }
        }
    }
    return paths_result;
}

void add_route(Route_array *array, Route *route)
{
    assert(array);
    
    array->size++;
    array->routes = realloc(array->routes, array->size * sizeof(Route*));
    
    if(array->routes == NULL)
        errx(EX_OSERR, "Out of memory");
    
    array->routes[array->size - 1] = route;
}

int node_in_route(int node, Route *route)
{
    int i;

    for(i = 0; i < route->trace_length; i++)
        if(route->trace[i] == node)
            return 1;

    return 0;
}

void make_weight_matrix()
{
    weights[0] = weights_0;
    weights[1] = weights_1;
    weights[2] = weights_2;
    weights[3] = weights_3;
    weights[4] = weights_4;
    weights[5] = weights_5;
    weights[6] = weights_6;
    weights[7] = weights_7;
    weights[8] = weights_8;
    weights[9] = weights_9;
    weights[10] = weights_10;
    weights[11] = weights_11;
}
