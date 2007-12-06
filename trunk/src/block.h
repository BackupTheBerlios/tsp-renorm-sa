#ifndef BLOCK_H
#define BLOCK_H

#include <math.h>

#include "tsp.h"

#define X_MARGIN 0.05
#define Y_MARGIN X_MARGIN

/* Define the value which is returned when no city is in the block. */
#define NO_CITY -1
#define MANY_CITIES -2
#define INIT_INDEX -3

/* Sets the rotation of the blocks. */
extern double rotation;

typedef struct {
	int *block_cty;
	int *block_idx;
	int filled_blocks;
	unsigned int length;
	unsigned int height;
} grd;

/* Create a sparse grid consisting of *length by *height fields. */
grd *create_grd(const unsigned int *length, const unsigned int *height);
/* Free a grid object. */
void free_grd(grd *grid);
/* 
 * Returns NO_CITY if no city is present in the grid at (*x, *y)
 * Returns MANY_CITIES if more than one city is present in the grid at (*x, *y)
 * Returns the city number if one city is present in the grid at (*x, *y)
 */
int has_city(grd *grid, int *x, int *y);

/*
 * The following two functions can be used to print the boxes which represent 
 * the grid. The functions save all the data in a space seperated format with
 * a header. These files can be read with R (see cran.r-project.org) and 
 * plotted. Using commands like this:
 *
 * # Read the cities saved with the function print_cities()
 * cities<-read.table("cities",header=T)
 * # Plot the cities.
 * plot(cities$city_x, cities$city_y,axes=F)
 * 
 * # Read the lines which represent the grid. This file is saved 
 * # using print_grd_lines()
 * lines<-read.table("lines",header=T)
 * # Plot the lines.
 * lines(as.vector(lines))
 *
 * # Read the center dots of each box. If no cities are in the box the point
 * # is a diamond, if there is one city is a solid circle. In the
 * # case there are multiple cities the centre is a triangle with point up.
 * points<-read.table("points",header=T)
 * # Plot the points.
 * points(points$x, points$y, pch=points$pch)
 */
void print_grd_lines(grd *grid, FILE *f);
void print_grd_points(grd *grid, FILE *f);
void print_cities(FILE *f);

#endif /* BLOCK_H */
