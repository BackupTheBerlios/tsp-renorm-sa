#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <math.h>

#include "tsp.h"
#include "io.h"

int shortest_routes[9][9][16];

void renormalize(Tsp *tsp)
{
    int cells_x, cells_y;
    int i, x, y, unity;
    double min_x, min_y;
    double max_x, max_y;
    double range_x, range_y;
    int ind_x, ind_y;
    int **cells = NULL;
    int **route_new = NULL;
    int **route_prev = NULL;
    
    cells_x = 2;
    cells_y = 2;
    
    min_x = max_x = result->cities[0].x;
    min_y = max_y = result->cities[0].y;
    for (i = 1; i < tsp->dimension; i++) {
        if (result->cities[i].x < min_x)
            min_x = result->cities[i].x;
        if (result->cities[i].x > max_x)
            max_x = result->cities[i].x;
        if (result->cities[i].y < min_y)
            min_y = result->cities[i].y;
        if (result->cities[i].y < max_y)
            min_x = result->cities[i].y;
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
        for (i = 0; i < result->dimension; i++) {
            ind_x = (int)floor(result->cities[i].x - min_x / range_x);
            ind_y = (int)floor(result->cities[i].y - min_y / range_y);
            
            if (cells[ind_x][ind_y])
                unity = 0;
                
            cells[ind_x][ind_y] = 1;
        }
        
        //Generate a route between the visited cells
        if ((route_new = calloc(cells_x / 2, sizeof(int*))) == NULL)
                errx(EX_OSERR, "Out of memory");
        
        for (i = 0; i < cells_x / 2; i++) 
            if ((route_new[i] = calloc(cells_y / 2, sizeof(int))) == NULL)
                errx(EX_OSERR, "Out of memory");
        
        for (x = 0; x < cells_x  / 2; x++) {
            for (y = 0; y < cells_y / 2; y++) {
                if (route_prev) {
                    //...
                }
                else {
                    route_new = getRoute(cells[x * 2][y * 2], 
                                         cells[x * 2][y * 2 + 1],
                                         cells[x * 2 + 1][y * 2],
                                         cells[x * 2 + 1][y * 2 + 1]);
                }
            }
        }
    }
}

int getRoute(int cell_topleft, int cell_topright,
             int cell_bottomleft, int cell_bottomright,
             int route_part, int route)
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
    int i;
    int start, end;
    int route[4];
    int route_size;
    
    for (start = 0; start < 9; start++) {
        for (end = 0; end < 9; end++) {
            for (
            if (start == end)
                
        
    }
}
