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

static void node_offset(int node, double *x, double *y);
static int point_on_edge(int edge_start, int edge_finish);

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

Route  *_basic_start = NULL;
Route  *_shortest_routes[BORDER_NODES][BORDER_NODES][BIT_CELL_MAX];
static int *_result;

/*
 * Function which solves the Symmetric TSP by using renormalization technique
 */
int    *
renormalize()
{
   int     cells_x, cells_y;
   int     t, l, unity;

   int     start, end;
   int     new_x, new_y;
   int     location;
   int     cells_v;

   int     ind_x, ind_y;

   grd    *grid;

   int     size = 256;
   Block  *block_a = NULL;
   Block  *block_b = NULL;

   Block  *block_new;

   int     prev_is_a = 0;

   int     new_ind;
   int     ind_city = 0;

   Route  *route;

   if ((_result = calloc(tsp->dimension, sizeof(int))) == NULL)
      errx(EX_OSERR, "Out of memory!");

   /*
    * Set up grid 
    */
   cells_x = 2;
   cells_y = 2;
   if ((block_a = realloc(block_a, size * sizeof(Block))) == NULL)
      errx(EX_OSERR, "Out of memory!");
   if ((block_b = realloc(block_b, size * sizeof(Block))) == NULL)
      errx(EX_OSERR, "Out of memory!");

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
      /*
       * Generate Cartesian grid, should also be done by a help function
       * (here for testing purposes)
       */
      grid = create_grd(&cells_x, &cells_y);
      printf("Iteration %d\n", cells_x);

      /*
       * Check if we are alreadqy at unity 
       */
      unity = 1;
      for (ind_x = 0; ind_x < cells_x; ind_x++) {
         for (ind_y = 0; ind_y < cells_y; ind_y++) {
            if (has_city(grid, ind_x, ind_y) == MANY_CITIES)
               unity = 0;
         }
      }

      /*
       * It is the first iteration, so entry and deperature points in a
       * block are not an issue yet
       */
      if (cells_x == 2) {
         cells_v = bitmask(grid, 0, 0);
         block_a[0].route = get_basic_route(cells_v);
         block_a[0].x = 0;
         block_a[0].y = 0;

         block_a[1].route = NULL;
      } else {
         new_ind = 0;
         for (t = 0; t < size; t++) {
            if (prev_is_a) {
               route = block_a[t].route;
               block_new = &(block_b[new_ind]);
            } else {
               route = block_b[t].route;
               block_new = &(block_a[new_ind]);
            }

            if (!route) {
               if (!unity)
                  block_new->route = NULL;
               break;
            }
            for (l = 0; l < CELL_NODES; l++) {
               if (prev_is_a) {
                  new_x = 2 * block_a[t].x;
                  new_y = 2 * block_a[t].y;

                  block_new = &(block_b[new_ind]);
               } else {
                  new_x = 2 * block_b[t].x;
                  new_y = 2 * block_b[t].y;

                  block_new = &(block_a[new_ind]);
               }

               location = route->visits[l];
               if (location == -1)
                  break;

               if (location % 2)
                  new_x++;
               if (location > 1)
                  new_y++;

               cells_v = bitmask(grid, new_x * 2, new_y * 2);

               start = route->start[location];
               end = route->end[location];

               assert(start != -1 && end != -1);

               block_new->route =
                   _shortest_routes[start - CELL_NODES][end - CELL_NODES]
                   [cells_v];
               block_new->x = new_x;
               block_new->y = new_y;

               if (unity)
                  map_block_on_route(block_new, grid, &ind_city);

               new_ind++;
               if (new_ind >= size) {
                  size *= 2;
                  if ((block_a =
                       realloc(block_a, size * sizeof(Block))) == NULL)
                     errx(EX_OSERR, "Out of memory!");
                  if ((block_b =
                       realloc(block_b, size * sizeof(Block))) == NULL)
                     errx(EX_OSERR, "Out of memory!");
               }
            }
         }
      }
      /*
       * Store iteration 
       */
/*        char name[32];
        sprintf(name, "/tmp/it%d", cells_x);
        FILE* f = fopen(name, "w");
        if (prev_is_a)
            print_routes(block_b, size, f);
        else
            print_routes(block_a, size, f);
        fclose(f);*/
      prev_is_a = !prev_is_a;
      free_grd(grid);
      grid = NULL;

      cells_x *= 2;
      cells_y *= 2;
   }

   free(block_a);
   free(block_b);
   return _result;
}

void
map_block_on_route(Block * block, grd * grid, int *ind)
{
   int     i;
   int     ind_x = block->x;
   int     ind_y = block->y;

   for (i = 0; i < block->route->trace_length; i++) {
      switch (block->route->trace[i]) {
      case NODE_CELL_TL:
         if (has_city(grid, 2 * ind_x, 2 * ind_y) != NO_CITY) {
            _result[*ind] = has_city(grid, 2 * ind_x, 2 * ind_y);
            *ind = *ind + 1;
         }
         break;
      case NODE_CELL_TR:
         if (has_city(grid, 2 * ind_x + 1, 2 * ind_y) != NO_CITY) {
            _result[*ind] = has_city(grid, 2 * ind_x + 1, 2 * ind_y);
            *ind = *ind + 1;
         }
         break;
      case NODE_CELL_BL:
         if (has_city(grid, 2 * ind_x, 2 * ind_y + 1) != NO_CITY) {
            _result[*ind] = has_city(grid, 2 * ind_x, 2 * ind_y + 1);
            *ind = *ind + 1;
         }
         break;
      case NODE_CELL_BR:
         if (has_city(grid, 2 * ind_x + 1, 2 * ind_y + 1) != NO_CITY) {
            _result[*ind] = has_city(grid, 2 * ind_x + 1, 2 * ind_y + 1);
            *ind = *ind + 1;
         }
         break;
      }
   }
}

/*
 * Convert visited cells into a bitmask
 */
int
bitmask(grd * grid, int ind_x, int ind_y)
{
   int     mask = 0;

   if (has_city(grid, ind_x, ind_y) != NO_CITY)
      mask |= BIT_CELL_TL;
   if (has_city(grid, ind_x + 1, ind_y) != NO_CITY)
      mask |= BIT_CELL_TR;
   if (has_city(grid, ind_x, ind_y + 1) != NO_CITY)
      mask |= BIT_CELL_BL;
   if (has_city(grid, ind_x + 1, ind_y + 1) != NO_CITY)
      mask |= BIT_CELL_BR;
   return mask;
}

/*
 * Get the basic route. A basic route is a case where no entry point and
 * departure point are specified on the edge of the square. For each cell
 * is specified if it needs to be visited or not using the arguments.
 * The basic route is a closed path connecting the selected points
 */
Route  *
get_basic_route(int cells)
{
   int     i;
   int     previous_point = -1;

   if (!_basic_start) {
      if ((_basic_start = calloc(1, sizeof(Route))) == NULL)
         errx(EX_OSERR, "Out of memory!");
   } else {
      _basic_start->trace_length = 0;
      _basic_start->length = 0.0;

      for (i = 0; i < CELL_NODES; i++) {
         _basic_start->visits[i] = -1;
         _basic_start->start[i] = -1;
         _basic_start->end[i] = -1;
      }

      for (i = 0; i < NODES; i++)
         _basic_start->trace[i] = -1;
   }

   /*
    * Simply visit all cells and you have already the shortest path 
    */
   if (cells & BIT_CELL_TL) {
      _basic_start->trace[_basic_start->trace_length] = NODE_CELL_TL;
      _basic_start->trace_length++;

      previous_point = NODE_CELL_TL;
   }
   if (cells & BIT_CELL_TR) {
      if (previous_point != -1) {
         _basic_start->trace[_basic_start->trace_length] =
             point_on_edge(previous_point, NODE_CELL_TR);
         _basic_start->trace_length++;
      }
      _basic_start->trace[_basic_start->trace_length] = NODE_CELL_TR;
      _basic_start->trace_length++;

      previous_point = NODE_CELL_TR;
   }
   if (cells & BIT_CELL_BR) {
      if (previous_point != -1) {
         _basic_start->trace[_basic_start->trace_length] =
             point_on_edge(previous_point, NODE_CELL_BR);
         _basic_start->trace_length++;
      }
      _basic_start->trace[_basic_start->trace_length] = NODE_CELL_BR;
      _basic_start->trace_length++;

      previous_point = NODE_CELL_BR;
   }
   if (cells & BIT_CELL_BL) {
      if (previous_point != -1) {
         _basic_start->trace[_basic_start->trace_length] =
             point_on_edge(previous_point, NODE_CELL_BL);
         _basic_start->trace_length++;
      }
      _basic_start->trace[_basic_start->trace_length] = NODE_CELL_BL;
      _basic_start->trace_length++;
   }
   /*
    * Repeat last node to close the path (If the path is longer than two) and
    * set start and endpoints in subblock
    */
   if (_basic_start->trace_length > 3) {
      previous_point =
          point_on_edge(_basic_start->trace[_basic_start->
                                            trace_length - 1],
                        _basic_start->trace[0]);

      for (i = _basic_start->trace_length; i > 0; i--)
         _basic_start->trace[i] = _basic_start->trace[i - 1];
      _basic_start->trace_length++;

      _basic_start->trace[0] = previous_point;
      _basic_start->trace[_basic_start->trace_length] = previous_point;
      _basic_start->trace_length++;

      set_borderpoints_subblocks(_basic_start);
   } else {
      errx(EX_DATAERR, "Try other grid range!\n");
   }
   return _basic_start;
}

/*
 * Initialize weight matrix of the default graph
 * Function is needed because this can not be done statically
 */
void
make_weight_matrix()
{
   int     i, j;
   //Initialize default on zero
   for (i = 0; i < NORMAL_NODES; i++)
      for (j = 0; j < NORMAL_NODES; j++)
         _weights[i][j] = 0.0;

   _weights[NODE_BORDER_TL][NODE_CELL_TL] = 0.707;

   _weights[NODE_BORDER_T][NODE_CELL_TL] = 0.707;
   _weights[NODE_BORDER_T][NODE_CELL_TR] = 0.707;

   _weights[NODE_BORDER_TR][NODE_CELL_TR] = 0.707;

   _weights[NODE_BORDER_L][NODE_CELL_TL] = 0.707;
   _weights[NODE_BORDER_L][NODE_CELL_BL] = 0.707;

   _weights[NODE_BORDER_R][NODE_CELL_TR] = 0.707;
   _weights[NODE_BORDER_R][NODE_CELL_BR] = 0.707;

   _weights[NODE_BORDER_BL][NODE_CELL_BL] = 0.707;

   _weights[NODE_BORDER_B][NODE_CELL_BR] = 0.707;
   _weights[NODE_BORDER_B][NODE_CELL_BL] = 0.707;

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
void
preprocess_routes()
{
   int     cells;
   int     start, end;
   int     i, j;
   int     paths_possible;
   int     visit_topleft, visit_topright;
   int     visit_bottomleft, visit_bottomright;
   int     is_shorter;
   Route_array path;
   Route  *shortest;

   make_weight_matrix();

   /*
    * Iterate over all start and endpoints in the graph 
    */
   for (start = 0; start < BORDER_NODES; start++) {
      for (end = 0; end < BORDER_NODES; end++) {
         path = paths(start + CELL_NODES, end + CELL_NODES, 0);

         /*
          * Decide for each possible combination of visited cells, what the
          * shortest route is
          */
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
               set_borderpoints_subblocks(_shortest_routes[start][end]
                                          [cells]);
            }
         }
         for (i = 0; i < path.size; i++)
            free(path.routes[i]);
         free(path.routes);
      }
   }
}

void
free_routes()
{
   int     start, end;
   int     cells;

   /*
    * Iterate over all start and endpoints in the graph 
    */
   for (start = 0; start < BORDER_NODES; start++)
      for (end = 0; end < BORDER_NODES; end++)
         for (cells = 0; cells < BIT_CELL_MAX; cells++)
            if (_shortest_routes[start][end][cells])
               free(_shortest_routes[start][end]
                    [cells]);
}

/*
 * Finds entry and departure blocks in one route block
 */
void
set_borderpoints_subblocks(Route * route)
{
   int     i;
   int     start_cur, end_cur;
   int     visits_no = 0;
   int     cell_a, cell_b;
   int     has_start = 0;
   int     alternative[CELL_NODES];
   int     location[CELL_NODES];

   if (!route)
      return;

   for (i = 0; i < CELL_NODES; i++) {
      route->visits[i] = -1;
      route->start[i] = -1;
      route->end[i] = -1;
   }
   for (i = 0; i < route->trace_length; i++) {
      if (route->trace[i] >= NODE_CELL_TL && route->trace[i] <= NODE_CELL_BR)
         continue;

      if (!has_start) {
         start_cur = i;
         has_start = 1;
      } else {
         end_cur = i;
         get_cell_index(route, start_cur, end_cur, &cell_a, &cell_b);

         if (route->start[cell_a] == -1) {
            route->start[cell_a] = route->trace[start_cur];
            route->end[cell_a] = route->trace[end_cur];

            location[cell_a] = visits_no;
            alternative[cell_a] = cell_b;

            visits_no++;
         } else if (route->start[cell_b] == -1) {
            route->start[cell_b] = route->trace[start_cur];
            route->end[cell_b] = route->trace[end_cur];

            location[cell_a] = visits_no;
            alternative[cell_b] = cell_b;

            visits_no++;
         } else if (alternative[cell_a]) {
            route->start[alternative[cell_a]] = route->start[cell_a];
            route->end[alternative[cell_a]] = route->end[cell_a];

            location[alternative[cell_a]] = location[cell_a];

            route->start[cell_a] = route->trace[start_cur];
            route->end[cell_a] = route->trace[end_cur];

            location[cell_a] = visits_no;
            visits_no++;
         } else if (alternative[cell_b]) {
            route->start[alternative[cell_b]] = route->start[cell_b];
            route->end[alternative[cell_b]] = route->end[cell_b];

            location[alternative[cell_b]] = location[cell_b];

            route->start[cell_b] = route->trace[start_cur];
            route->end[cell_b] = route->trace[end_cur];

            location[cell_b] = visits_no;
            visits_no++;
         }
         start_cur = end_cur;
      }
   }

   for (i = 0; i < CELL_NODES; i++) {
      route->start[i] = convert_node(i, route->start[i]);
      route->end[i] = convert_node(i, route->end[i]);

      if (route->start[i] != -1 && route->end[i] != -1)
         route->visits[location[i]] = i;
   }
}

/*
 * Function giving all possible paths between start and end, which visit each
 * node maximal one time. It results a list of pointers to the found routes
 */
Route_array
paths(int start, int end, int visited)
{
   Route_array routes_new;
   Route_array routes_cur;
   Route  *route;
   int     paths_previous_no, i, j, k;
   int     node_between;
   int     diff;

   routes_new.routes = NULL;
   routes_new.size = 0;

   /*
    * Function wants all the routes with the same start and endpoint for
    * calculating the shortest path, Normally you would aspect that this path
    * has a length of 0 (Stay at the location). However maybe he needs to
    * visit some cells
    */
   if (start == end && visited == 0) {
      /*
       * Find edges out of the start point. Use the destination of these
       * edge as a start point for a route to our endpoint, refuse routes
       * which use the same edge we have selected, since it is not allowed
       * to use an edge twice
       */
      for (i = 0; i < NORMAL_NODES; i++) {
         if (_weights[i][end]) {
            routes_cur = paths(i, end, 0);
            node_between = point_on_edge(i, end);

            for (j = 0; j < routes_cur.size; j++) {
               route = routes_cur.routes[j];

               /*
                * Route is one edge long uses this discovered edge 
                */
               if (route->trace_length == 2) {
                  free(route);
                  continue;
               } else if (route->trace_length == 3 &&
                          route->trace[0] == start &&
                          route->trace[1] ==
                          node_between && route->trace[2] == i) {
                  free(route);
                  continue;
               }
               diff = node_between == -1 ? 1 : 2;

               /*
                * Put our startpoint in front again to make round tour 
                */
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
   } else if (start == end) {
      /*
       * Base case: all paths from a point to itself. Since we are
       * eventually interested in shortest paths, there is actually one path
       * (stay at your location)
       */
      if ((route = calloc(1, sizeof(Route))) == NULL)
         errx(EX_OSERR, "Out of memory");

      route->trace[0] = start;
      route->trace_length = 1;
      route->length = 0.0;
      add_route(&routes_new, route);
   } else {
      /*
       * For getting all possible paths you search all connected neighbours.
       * From here the paths are recursively determined. Now you only need
       * to add this current point to the trace. Do not visit neighbours
       * which are already visited in the current route, since this would
       * produce cycles in the route
       */
      for (i = 0; i < NORMAL_NODES; i++) {
         if (_weights[i][end] && !(visited & (1 << i))) {
            routes_cur = paths(start, i, visited | (1 << end));
            node_between = point_on_edge(i, end);

            for (j = 0; j < routes_cur.size; j++) {
               route = routes_cur.routes[j];

               /*
                * Edge this edge to the route (With the node in between) 
                */
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
void
add_route(Route_array * array, Route * route)
{
   assert(array);

   array->size++;
   array->routes = realloc(array->routes, array->size * sizeof(Route *));

   if (array->routes == NULL)
      errx(EX_OSERR, "Out of memory");

   array->routes[array->size - 1] = route;
}

/*
 *  Copy a route
 */
Route  *
copy_route(Route * src)
{
   int     i;
   Route  *dest;

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
int
node_in_route(int node, Route * route)
{
   int     i;

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
int
route_visits_cells(Route * route, int cells)
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
static int
point_on_edge(int edge_start, int edge_finish)
{
   int     swap;

   if (edge_start > edge_finish) {
      swap = edge_start;
      edge_start = edge_finish;
      edge_finish = swap;
   }
   if (edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_TR)
      return NODE_CROSS_T;
   if (edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_BL)
      return NODE_CROSS_L;
   if (edge_start == NODE_CELL_TL && edge_finish == NODE_CELL_BR)
      return NODE_CROSS_C;
   if (edge_start == NODE_CELL_TR && edge_finish == NODE_CELL_BL)
      return NODE_CROSS_C;
   if (edge_start == NODE_CELL_TR && edge_finish == NODE_CELL_BR)
      return NODE_CROSS_R;
   if (edge_start == NODE_CELL_BL && edge_finish == NODE_CELL_BR)
      return NODE_CROSS_B;

   return -1;
}

void
get_corresponding_cell(int point, int *cell_a, int *cell_b)
{
   *cell_a = -1;
   *cell_b = -1;

   switch (point) {
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

void
get_cell_index(Route * route, int start, int end, int *cell_a, int *cell_b)
{
   int     start_in_cell[CELL_NODES];
   int     end_in_cell[CELL_NODES];
   int     is_path;
   int     i, j;

   for (i = 0; i < CELL_NODES; i++) {
      start_in_cell[i] = 0;
      end_in_cell[i] = 0;
   }

   *cell_a = -1;
   *cell_b = -1;

   get_corresponding_cell(route->trace[start], cell_a, cell_b);
   if (*cell_a == ANY_NODE_CELL)
      for (i = 0; i < CELL_NODES; i++)
         start_in_cell[i] = 1;
   else if (*cell_a != -1)
      start_in_cell[*cell_a] = 1;

   if (*cell_b != -1)
      start_in_cell[*cell_b] = 1;

   get_corresponding_cell(route->trace[end], cell_a, cell_b);
   if (*cell_a == ANY_NODE_CELL)
      for (i = 0; i < CELL_NODES; i++)
         end_in_cell[i] = 1;
   else if (*cell_a != -1)
      end_in_cell[*cell_a] = 1;

   if (*cell_b != -1)
      end_in_cell[*cell_b] = 1;

   *cell_a = -1;
   *cell_b = -1;

   for (i = 0; i < CELL_NODES; i++) {
      if (start_in_cell[i] && end_in_cell[i]) {
         is_path = 1;
         for (j = start + 1; j < end; j++)
            if (route->trace[j] != i)
               is_path = 0;

         if (!is_path)
            continue;

         if (*cell_a != -1)
            *cell_b = i;
         else
            *cell_a = i;
      }
   }
}

int
convert_node(int location, int main_node)
{
   switch (location) {
   case NODE_CELL_TL:
      switch (main_node) {
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
      switch (main_node) {
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
      switch (main_node) {
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
      switch (main_node) {
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

static void
node_offset(int node, double *x, double *y)
{
   *x = 0.0;
   *y = 0.0;

   switch (node) {
   case NODE_BORDER_TL:
   case NODE_BORDER_L:
   case NODE_BORDER_BL:
      *x = -0.5;
      break;
   case NODE_CELL_TL:
   case NODE_CELL_BL:
   case NODE_CROSS_L:
      *x = -0.25;
      break;
   case NODE_CELL_TR:
   case NODE_CELL_BR:
   case NODE_CROSS_R:
      *x = 0.25;
      break;
   case NODE_BORDER_TR:
   case NODE_BORDER_R:
   case NODE_BORDER_BR:
      *x = 0.5;
      break;
   }

   switch (node) {
   case NODE_BORDER_TL:
   case NODE_BORDER_T:
   case NODE_BORDER_TR:
      *y = -0.5;
      break;
   case NODE_CELL_TL:
   case NODE_CELL_TR:
   case NODE_CROSS_T:
      *y = -0.25;
      break;
   case NODE_CELL_BL:
   case NODE_CELL_BR:
   case NODE_CROSS_B:
      *y = 0.25;
      break;
   case NODE_BORDER_BL:
   case NODE_BORDER_B:
   case NODE_BORDER_BR:
      *y = 0.5;
      break;
   }
}

void
print_routes(Block * blocks, int max_size, FILE * f)
{
   double  offset_x, offset_y;
   double  base_x, base_y;
   double  x, y;
   double  width_x, width_y;

   Route  *route;

   int     t, l;

   width_x = 2.0;
   width_y = 2.0;

   /*
    * Print the header. 
    */
   (void) fprintf(f, "x y\n");

   for (t = 0; t < max_size; t++) {
      route = blocks[t].route;
      base_x = blocks[t].x * width_x + 0.5 * width_x;
      base_y = blocks[t].y * width_y + 0.5 * width_y;

      if (!route)
         break;

      for (l = 0; l < route->trace_length; l++) {
         node_offset(route->trace[l], &offset_x, &offset_y);
         x = base_x + offset_x * width_x;
         y = base_y + offset_y * width_y;
         (void) fprintf(f, "%lf %lf\n", x, y);
      }
   }
   (void) fprintf(f, "NA NA\n");
}
