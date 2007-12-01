#ifndef DISTANCE_H
#define DISTANCE_H

/*
 * Compute the distance between cities.
 *
 * route The cities among which the distance should be computed. This
 *			is an array of ints.
 * num_cities The number of cities in the route.
 */
double route_length(const int *route, int num_cities);

#endif
