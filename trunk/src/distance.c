#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include "tsp.h"
#include "distance.h"

double 
route_length(const int *route, int num_cities)
{
	assert(num_cities != 0);
	assert(route != NULL);
	assert(tsp != NULL);
	assert(tsp->distance_type == EUC_2D);

	double length;
	double height;
	double route_lngth = 0;
	City *city_curr, *city_nxt;

	for (unsigned int i = 0; i < (num_cities - 1); i++) {
		assert(route[i] < tsp->dimension);
	
		/* Temporary assign the cities to readable variables. */
		city_curr = &tsp->cities[route[i]];
		city_nxt = &tsp->cities[route[i + 1]];
		
		/* Cities should be different. */
		assert(city_curr != city_nxt);

		/* Use Pythagoras law to compute the distance. */
		length = city_curr->x - city_nxt->x;
		height = city_curr->y - city_nxt->y;
		route_lngth += sqrt(length * length + height * height);
	}

	return route_lngth;
}
