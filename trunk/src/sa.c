#include <math.h>
#include <stdlib.h>
#include <err.h>

#include "sa.h"
#include "tsp.h"
#include "renormalization.h"
#include "distance.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

static void neighbour_rot(double temp, double temp_end, double temp_init);

double
thermo_sa(double initstate, double temp_end, double temp_init, double temp_sig)
{
	double energy, energy_new, energy_delta, energy_variation; 
	int *path;
	int i;
	double temp, temp_old;
	double prob;
	double rot_old, best_rot;
	double entropy_variation;
	double best_energy = 10000000000;
	double route_len = 0;
	double rotation;

	temp = temp_init;
	rotation = initstate;

   path = renormalize();
   energy = route_length(path, tsp->dimension);
	warnx("Energy %lf", energy);
	
	entropy_variation = 0;
	energy_variation = 0;


	i = 1.1;
	do {
		temp_old = temp;
		rot_old = rotation;
		neighbour_rot(temp, temp_end, temp_init);
		path = renormalize();
		//energy_new = route_length(path, tsp->dimension);
		route_len = route_length(path, tsp->dimension);
		/* The cost function is too bumpy by itself. So dividing it by the
		 * number of iterations causes it to be bumpy only at the beginning. */
		energy_new = route_len * 1.0f / log((double)i);
		warnx("energy_new %lf energy %lf", energy_new, energy);
		energy_delta = energy_new - energy;
		prob = exp(-energy_delta / temp);

		if (drand48() < prob) {
			if (best_energy > route_len) {
				best_energy = route_len;
				best_rot = rotation;
			}
			energy = energy_new;
			energy_variation += energy_delta;
		} else {
			rotation = rot_old;
		}
		if (energy_delta < 0) {
			entropy_variation -= energy_delta / temp;
		}

		if ((energy_variation >= 0) || fabs(entropy_variation) < 0.000001)  {
			temp = temp_init;
		} else {
			warnx("Set energy_var %lf  entropy_var %lf temp_old %lf new temp %lf", energy_variation, entropy_variation, temp,i * (-energy_variation / entropy_variation));
			temp = i * (energy_variation / entropy_variation);
		}
		i++;
		warnx("temp %lf temp_old %lf energy_new %lf energy_delta %lf energy_variation %lf",
				temp, temp_old, energy_new, energy_delta, energy_variation);
		warnx("entropy_variation %lf prob %lf rot %lf best %lf best rot %lf",
				entropy_variation, prob, rotation, best_energy, best_rot);
	} while ((temp > temp_end) || (fabs(temp - temp_old) > temp_sig));

	return best_energy;
}

void
neighbour_rot(double temp, double temp_end, double temp_init)
{
	const double rot_min = 0.01 * M_PI;
	const double rot_max = 2 * M_PI;

	const double rot_lim = rot_min + (rot_max - rot_min) / 
		(temp_init - temp_end) * (temp - temp_end);

	rotation = rot_lim * drand48();
}

