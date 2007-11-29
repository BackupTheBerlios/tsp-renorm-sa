#ifndef BLOCK_H
#define BLOCK_H

#include <math.h>

#include "tsp.h"

#define Y_OFFSET 0.001
#define X_OFFSET Y_OFFSET

/* Define the value which is returned when no city is in the block. */
#define NO_CITY -1
#define MANY_CITIES -2
/* Determine if the returned value is a city. */
#define has_city(x) ((x) != NO_CITY)
#define has_cities(x) ((x) != MANY_CITIES)


/* Sets the rotation of the blocks. */
extern double rotation;

typedef struct {
	int **block;
	unsigned int length;
	unsigned int height;
} grd;

/* */
grd *create_grd(const unsigned int *length, const unsigned int *height);
void free_grd(grd *grid);

#endif /* BLOCK_H */
