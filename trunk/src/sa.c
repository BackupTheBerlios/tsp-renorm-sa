#include <math.h>
#include <stdlib.h>
#include <err.h>

#include "sa.h"
#include "tsp.h"
#include "renormalization.h"
#include "block.h"
#include "distance.h"

static void neighbour_rot(double temp, double temp_end, double temp_init);

double
thermo_sa(double initstate, double temp_end, double temp_init, double temp_sig)
{
	double energy, energy_new, energy_delta, energy_variation; 
	int *path;
	int i;
	double temp, temp_old;
	double prob;
	double rot_opt;
	double entropy_variation;
	double best_energy;

	temp = temp_init;
	rotation = initstate;

   path = renormalize();
   energy = route_length(path, tsp->dimension);
	warnx("Energy %lf", energy);
	
	entropy_variation = 0;
	energy_variation = 0;


	i = 1;
	do {
		temp_old = temp;
		neighbour_rot(temp, temp_end, temp_init);
		path = renormalize();
		energy_new = route_length(path, tsp->dimension);
		energy_delta = energy_new - energy;
		energy = energy_new;
		prob = exp(-energy_delta / temp);
		if (drand48() < prob) {
			best_energy = energy_new;
			rot_opt = rotation;
			energy_variation += energy_delta;
		}
		if (energy_delta < 0) {
			entropy_variation -= energy_delta / temp;
		}

		if ((energy_variation >= 0) || fabs(entropy_variation) < 0.000001)  {
			temp = temp_init;
		} else {
			temp = i * (energy_variation / entropy_variation);
		}
		i++;
		warnx("temp %lf temp_old %lf energy_new %lf energy_delta %lf energy_variation %lf entropy_variation %lf prob %lf rot %lf best %lf",
				temp, temp_old, energy_new, energy_delta, energy_variation, entropy_variation, prob, rotation, best_energy);
	} while ((temp > temp_end) || (fabs(temp - temp_old) > temp_sig));

	return rot_opt;
}

void
neighbour_rot(double temp, double temp_end, double temp_init)
{
	const double rot_min = 0.01 * M_PI;
	const double rot_max = 2 * M_PI;

	const double factor = (rot_max - rot_min) / (temp_init - temp_end);

	rotation = rot_min + factor * (temp - temp_end);
}
