#include <math.h>
#include <err.h>
#include <stdlib.h>
#include <sysexits.h>
#include <assert.h>
#include <stdio.h>

#include "block.h"
#include "tsp.h"

double extern rotation = 0;

/* This contains all the cached values. */
static double _rotation = FP_NAN;
static double _x_max;
static double _x_min;
static double _y_max;
static double _y_min;
static City *_rot_cities = NULL;
static unsigned int _num_cities;

static void check_limits(void);
static void rotate(void);

grd *
create_grd(const unsigned int *length, const unsigned int *height)
{
	/* Update the cache containing the rotated cities if necessary. */
	rotate();

	/* Create a new grid and initialize all the values. */
	grd *new_grd;

	if ((new_grd = calloc(1, sizeof(grd))) == NULL)
		errx(EX_OSERR, "Not enough memory!");

	new_grd->length = *length;
	new_grd->height = *height;

	if ((new_grd->block = calloc(*length, sizeof(int *))) == NULL)
		errx(EX_OSERR, "Not enough memory!");

	for (int i = 0; i < *length; i++) {
		if ((new_grd->block[i] = calloc(*height, sizeof(int))) == NULL)
			errx(EX_OSERR, "Not enough memory!");
		for (int j = 0; j < *height; j++)
			new_grd->block[i][j] = NO_CITY;
	}

	
	/* Compute the necessary values which will be used to index the cities. */
	double x_step = fabs(_x_min - _x_max) / (double)(*length);
	double y_step = fabs(_y_min - _y_max) / (double)(*height);

	/* Determine for each city in which block it should be added. If multiple
	 * cities are in one block, set the block to contain MANY_CITIES otherwise
	 * set the block value to the city number. */
	for (int i = 0; i < _num_cities; i++) {
		unsigned int x = rint(floor((_rot_cities[i].x - _x_min) / x_step));
		unsigned int y = rint(floor((_rot_cities[i].y - _y_min) / y_step));
		if (!has_city(new_grd->block[x][y]))
			new_grd->block[x][y] = i;
		else
			new_grd->block[x][y] = MANY_CITIES;
	}

	return new_grd;
}

void
print_cities(FILE *f)
{
	assert(f != NULL);
	(void)fprintf(f, "city_x city_y\n");
	for (int i = 0; i < _num_cities; i++)
		(void)fprintf(f, "%lf %lf\n", _rot_cities[i].x, _rot_cities[i].y);
}

void 
free_grd(grd *grid)
{
	assert(grid != NULL);

	for (int i = 0; i < grid->length; i++)
		free(grid->block[i]);

	free(grid);
	grid = NULL;
}


static void
rotate(void)
{
	/* It the old rotation is the same nothing has to be done. */
	if (_rotation != FP_NAN && _rotation == rotation) 
		return;

	/* Free and allocate memory for the cities in the rotated plane. */
	if (_rot_cities != NULL)
		free(_rot_cities);

	if ((_rot_cities = calloc(tsp->dimension, sizeof(City))) == NULL)
		errx(EX_OSERR, "No memory left!");

	/* Rotate the cities. */
	for (int i = 0; i < tsp->dimension; i++) {
		_rot_cities[i].x = tsp->cities[i].x * cos(rotation) + 
			tsp->cities[i].y * sin(rotation);
		_rot_cities[i].y = -tsp->cities[i].x * sin(rotation) + 
			tsp->cities[i].y * cos(rotation);
	}
	/* Update the cached cities. */
	_rotation = rotation;
	_num_cities = tsp->dimension;

	check_limits();
}

void
print_grd_lines(grd *grid, FILE *f)
{
	assert(grid != NULL);
	assert(f != NULL);
	/* Compute the necessary values which will be used to index the cities. */
	double x_step = fabs(_x_min - _x_max) / (double)(grid->length - 1);
	double y_step = fabs(_y_min - _y_max) / (double)(grid->height - 1);

	/* Print the grid. At the end of each line a NA is needed to halt the line.
	 * See for more information on this the R manual for lines(). */
	/* The header. */
	(void)fprintf(f, "x y\n");
	/* First the vertical lines. */
	for (double x = _x_min; x < _x_max; x += x_step) {
		(void)fprintf(f, "%lf %lf\n", x, _y_min);
		(void)fprintf(f, "%lf %lf\n", x, _y_max);
		(void)fprintf(f, "%lf NA\n");
	}
	(void)fprintf(f, "%lf %lf\n", _x_max, _y_min);
	(void)fprintf(f, "%lf %lf\n", _x_max, _y_max);
	(void)fprintf(f, "%lf NA\n");

	/* The horizontal lines. */
	for (double y = _y_min; y < _y_max; y += y_step) {
		(void)fprintf(f, "%lf %lf\n", _x_min, y);
		(void)fprintf(f, "%lf %lf\n", _x_max, y);
		(void)fprintf(f, "%lf NA\n");
	}
	(void)fprintf(f, "%lf %lf\n", _x_min, _y_max);
	(void)fprintf(f, "%lf %lf\n", _x_max, _y_max);
	(void)fprintf(f, "%lf NA\n");
}

void
print_grd_points(grd *grid, FILE *f)
{
	assert(grid != NULL);
	assert(f != NULL);
	/* Compute the necessary values which will be used to index the cities. */
	double x_step = fabs(_x_min - _x_max) / (double)(grid->length);
	double y_step = fabs(_y_min - _y_max) / (double)(grid->height);
	
	/* Print the header. */
	(void)fprintf(f, "x y pch\n");
	unsigned int x_i = 0;
	unsigned int y_i;
	for (double x = _x_min + 0.5 * x_step; x < _x_max; x += x_step) {
		y_i = 0;
		for (double y = _y_min + 0.5 * y_step; y < _y_max; y += y_step) {
			/* Print the coordinate of the center of a box. */
			(void)fprintf(f, "%lf %lf", x, y);
			/* Determine the logo which will represent the center of the box. 
			 * These numbers are R codes. */
			if (has_cities(grid->block[x_i][y_i]))
				/* pch=24 is a triangle point up. */
				(void)fprintf(f, " 24\n");
			else if (has_city(grid->block[x_i][y_i]))
				/* pch=19 is a solid circle. */
				(void)fprintf(f, " 19\n");
			else
				/* pch=23 is a diamond. */
				(void)fprintf(f, " 23\n");
			y_i++;
		}
		x_i++;
	}
}

static void
check_limits(void)
{
	if (_rot_cities == NULL)
		return;

	/* Cache the x-max and min and the y-max and min. */
	_x_max = -FP_INFINITE;
	_x_min = FP_INFINITE;
	_y_max = -FP_INFINITE;
	_y_min = FP_INFINITE;

	for (int i = 0; i < _num_cities; i++) {
		if (_rot_cities[i].x > _x_max) 
			_x_max = _rot_cities[i].x;
		if (_rot_cities[i].x < _x_min)
			_x_min = _rot_cities[i].x;
		if (_rot_cities[i].y > _y_max)
			_y_max = _rot_cities[i].y;
		if (_rot_cities[i].y < _y_min)
			_y_min = _rot_cities[i].y;
	}

	/* Built some margin to be sure that all the cities are included in 
	 * a box. */
	_x_max += X_MARGIN;
	_x_min -= X_MARGIN;
	_y_max += Y_MARGIN;
	_y_min -= Y_MARGIN;
}

