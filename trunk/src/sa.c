#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_rng.h>

#include "sa.h"
#include "tsp.h"
#include "renormalization.h"
#include "distance.h"

#ifndef M_PI
#define M_PI 3.14159265358979
#endif

/* Returns the Brownian motion used to change the rotation. */
static double neighbour_rot(double temp, double temp_end, double temp_init,
                            double bm_sigma);

gsl_rng *_bm_rng;

double
thermo_sa(double temp_init, double temp_end, double temp_sig, double initstate,
          double bm_sigma, double k, FILE * log)
{
   double  energy, energy_new, energy_delta, energy_variation;
   int    *path;
   double  temp, temp_old;
   double  prob;
   double  rot_old, best_rot;
   double  entropy_variation;
   double  energy_best;
   gsl_rng *acpt_rng;
   unsigned long time = 0;
   double  BM;

   /*
    * Initialize the random number generators. 
    */
   _bm_rng = gsl_rng_alloc(gsl_rng_taus);
   acpt_rng = gsl_rng_alloc(gsl_rng_taus);

   temp = temp_init;
   rotation = initstate;

   /*
    * Compute the first path. 
    */
   path = renormalize();
   energy = route_length(path, tsp->dimension);
	free(path);
   energy_best = energy;

   entropy_variation = 0;
   energy_variation = 0;

   /*
    * Print the log headers. 
    */
   if (log != NULL)
      (void) fprintf(log, "time T E_n E_d E_v E_b S_v rb r rv bm\n");

   do {
      temp_old = temp;
      rot_old = rotation;
      if (log != NULL)
         (void) fprintf(log, "%lu ", time);

      BM = neighbour_rot(temp, temp_end, temp_init, bm_sigma);

      if(fpclassify(rotation) == FP_NAN)
          errx(EX_DATAERR, "Rotation can not be NaN");
      path = renormalize();
      energy_new = route_length(path, tsp->dimension);
		free(path);
      energy_delta = energy_new - energy;

      prob = exp(-energy_delta / temp);

      if (log != NULL)
         (void) fputs("*", stdout);
      (void) fprintf(log, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
							temp, energy_new, energy_delta, energy_variation,
							energy_best, entropy_variation, best_rot, rotation,
                     (rotation - rot_old), BM);

      if (gsl_rng_uniform(acpt_rng) < prob) {
         if (energy_best > energy) {
            energy_best = energy;
            best_rot = rotation;
         }
         energy = energy_new;
         energy_variation += energy_delta;
      } else
         rotation = rot_old;

      if (energy_delta > 0)
         entropy_variation -= energy_delta / temp;

      if ((energy_variation >= 0) || fabs(entropy_variation) < 0.000001)
         temp = temp_init;
      else {
         temp = k * (energy_variation / entropy_variation);
         rotation = best_rot;
      }
      time++;
   } while ((temp > temp_end) || (fabs(temp - temp_old) > temp_sig));

   gsl_rng_free(acpt_rng);
   gsl_rng_free(_bm_rng);

   return energy_best;
}

double
neighbour_rot(double temp, double temp_end, double temp_init, double bm_sigma)
{
   const double BM_start = 2 * M_PI;

    double BM = BM_start *
       exp((bm_sigma * (temp - temp_end) /
           (temp_init - temp_end)) *
           gsl_cdf_gaussian_Pinv(gsl_rng_uniform(_bm_rng), 1));
   if (isinf(BM))
        BM = MAXFLOAT;

   rotation = fmod(fabs(rotation + BM), 2 * M_PI);

   return BM;
}
