#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <err.h>
#include <unistd.h>
#include <limits.h>
#include <getopt.h>

#include "tsp.h"
#include "io.h"
#include "distance.h"
#include "block.h"
#include "tsp.h"
#include "sa.h"
#include <config.h>

/*
 * Print usage information.
 */
static void usage(void);

Tsp    *tsp;

int
main(int argc, char *argv[])
{
   char    ch, *ep;
   int     iters = 0;
   FILE   *toimport = NULL;

   while ((ch = getopt(argc, argv, "f:i:?h")) != -1)
      switch (ch) {
      case 'f':
         if ((toimport = fopen(optarg, "r")) == NULL)
            errx(EX_DATAERR, "Unable to open file %s", optarg);
         break;
      case 'i':
         if ((iters = (int) strtol(optarg, &ep, 10)) <= 0)
            usage();
         break;
      case '?':
      case 'h':
      default:
         usage();
      }

   if (toimport == NULL)
      usage();

   argc -= optind;
   argv += optind;

	/* Load and initialize the tsp data set for the renormalization. */
   tsp = import_tsp(toimport);
   preprocess_routes();
   fclose(toimport);
   //rotation = 1.275337;
   //int* cities = renormalize();

	thermo_sa(0.06 * M_PI, 0.01, 2500, 0.0001);
	
	//for (rotation = -0.01 * M_PI; rotation < M_PI; rotation += 0.01 * M_PI) {
      //int* cities = renormalize();
      //printf("%f %f\n", rotation, route_length(cities, tsp->dimension));        
   //}

   return EX_OK;
}

static void
usage(void)
{
   (void) fprintf(stderr,
                  "usage tsp -f [filename] -i [number of iterations]\n");
   (void) fprintf(stderr, "-f [filename]              The filename from which \
the distances should be loaded.\n");
   (void) fprintf(stderr, "-i [number of iterations]  The number of iterations \
which should be used to compute the shortest path.\n");
   (void) fprintf(stderr, "\n");
   (void) fprintf(stderr, "Travelling salesman solver version %s.\n", VERSION);
   (void) fprintf(stderr, "Report bugs to %s.\n", PACKAGE_BUGREPORT);

   exit(EX_USAGE);
}
