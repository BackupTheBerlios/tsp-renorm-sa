#include <math.h>
#include <err.h>
#include <stdlib.h>
#include <sysexits.h>
#include <assert.h>

#include "block.h"
#include "tsp.h"

double extern rotation = 0;


static double _rotation = FP_INFINITE;
static double _x_max;
static double _x_min;
static double _y_max;
static double _y_min;
static City *_rot_cities = NULL;
static unsigned int _num_cities;

static void check_limits(void);
static void rotate(void);

grd *create_grd(const unsigned int *length, const unsigned int *height)
{
	/* Update the cache containing the rotated cities if necessary. */
	rotate();

	/* Create a new grid and initialize all the values. */
	grd *new_grd;

	if ((new_grd = malloc(sizeof(grd))) == NULL)
		errx(EX_OSERR, "Not enough memory!");

	if ((new_grd->block = calloc(*length, sizeof(int *))) == NULL)
		errx(EX_OSERR, "Not enough memory!");

	for (int i = 0; i < *length; i++) {
		if ((new_grd->block[i] = calloc(*height, sizeof(int))) == NULL)
			errx(EX_OSERR, "Not enough memory!");
		for (int j = 0; j < *height; j++)
			new_grd->block[i][j] = NO_CITY;
	}

	new_grd->length = *length;
	new_grd->height = *height;

	/* Compute the necessary values which will be used to index the cities. */
	double x_step = fabs(_x_min - _x_max) / (double)*length;
	double y_step = fabs(_y_min - _y_max) / (double)*height;

	/* Determine for each city in which block it should be added. If multiple
	 * cities are in one block, set the block to contain MANY_CITIES otherwise
	 * set the block value to the city number. */
	for (int i = 0; i < _num_cities; i++) {
		unsigned int x = rint(floor((_rot_cities[i].x - _x_min) / x_step));
		unsigned int y = rint(floor((_rot_cities[i].y - _y_min) / x_step));
		if (!has_city(new_grd->block[x][y]))
			new_grd->block[x][y] = i;
		else
			new_grd->block[x][y] = MANY_CITIES;
	}

	return new_grd;
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
	if (_rotation == rotation)
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
	_rotation = rotation;
	_num_cities = tsp->dimension;

	check_limits();
}

static void
check_limits(void)
{
	if (_rot_cities == NULL)
		return;

	_x_max = -FP_INFINITE;
	_y_max = -FP_INFINITE;
	_x_min = FP_INFINITE;
	_y_min = FP_INFINITE;

	for (int i = 0; i < _num_cities; i++) {
		if (_rot_cities[i].x > _x_max)
			_x_max = _rot_cities[i].x;
		if (_rot_cities[i].y > _y_max)
			_y_max = _rot_cities[i].y;
		if (_rot_cities[i].x < _x_min)
			_x_min = _rot_cities[i].x;
		if (_rot_cities[i].y > _y_min)
			_y_min = _rot_cities[i].y;
	}
}

