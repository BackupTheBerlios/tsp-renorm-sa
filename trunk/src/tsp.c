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

#ifndef M_PI
#define M_PI 3.14159265358979
#endif
/*
 * Print usage information.
 */
static void usage(void);

Tsp    *tsp;

int
main(int argc, char *argv[])
{
   char    ch, *ep;
	double  bm_sigma = 0.2, temp_end = 1, temp_init = 100, init_state = 0;
	double  k = 0;
   FILE   *toimport = NULL;
	FILE	 *log = NULL;

   while ((ch = getopt(argc, argv, "f:i:s:e:b:k:l:?h")) != -1)
      switch (ch) {
      case 'l':
         if ((log = fopen(optarg, "w")) == NULL)
            errx(EX_DATAERR, "Unable to open file %s", optarg);
         break;
      case 'f':
         if ((toimport = fopen(optarg, "r")) == NULL)
            errx(EX_DATAERR, "Unable to open file %s", optarg);
         break;
      case 'i':
         if ((init_state = strtod(optarg, &ep)) < 0)
            usage();
			init_state = fmod(init_state, 2 * M_PI);
         break;
		case 's':
         if ((bm_sigma = strtod(optarg, &ep)) <= 0)
            usage();
         break;
		case 'e':
         if ((temp_end = strtod(optarg, &ep)) <= 0)
            usage();
         break;
  		case 'b':
         if ((temp_init = strtod(optarg, &ep)) <= 0)
            usage();
         break;
  		case 'k':
         if ((k = strtod(optarg, &ep)) <= 0)
            usage();
         break;
		case '?':
      case 'h':
      default:
         usage();
      }

   if (toimport == NULL) {
		warnx("No import file!");
      usage();
	}
	if (temp_end > temp_init) {
		warnx("The end temperature must be smaller than the begin temperature.");
		usage();
	}

	warnx("Start solving with initial state %1.4lf begin temp %04.2lf \
end temp %04.2lf Brownian motion sigma %lf", init_state, temp_init, temp_end, 
		bm_sigma);

   argc -= optind;
   argv += optind;

	/* Load and initialize the tsp data set for the renormalization. */
   tsp = import_tsp(toimport);
   preprocess_routes();
   fclose(toimport);

	double energy = thermo_sa(temp_init, temp_end, 0.01, init_state, bm_sigma, 
			k, log);
	warnx("Best energy found %lf", energy);
	fclose(log);

   return EX_OK;
}

static void
usage(void)
{
   (void) fprintf(stderr,
                  "usage tsp -f [filename] -i [initstate] -s [BM sigma] \
-e [end temp] -b [begin temp] -l [log file]\n");
   (void) fprintf(stderr, "-f [filename]    The filename from which \
the distances should be loaded.\n");
   (void) fprintf(stderr, "-l [log file]    The filename where the \
TSA info should be stored in.\n");
   (void) fprintf(stderr, "-i [init state]  The inital state of TSA \
(default 0)\n");
   (void) fprintf(stderr, "-s [BM sigma]    The sigma of the Brownian \
motion (default 0.2)\n");
   (void) fprintf(stderr, "-b [begin temp]  The begin temperature \
of the TSA (default 100)\n");
   (void) fprintf(stderr, "-e [end temp]    The end temperature \
of the TSA (default 1)\n");
   (void) fprintf(stderr, "\n");
   (void) fprintf(stderr, "Travelling salesman solver version %s.\n", VERSION);
   (void) fprintf(stderr, "Report bugs to %s.\n", PACKAGE_BUGREPORT);

   exit(EX_USAGE);
}
