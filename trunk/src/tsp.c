#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <unistd.h>
#include <limits.h>

#include "tsp.h"
#include "io.h"
#include <config.h>

/*
 * Print usage information.
 */
static void usage(void);

int 
main(int argc, char *argv[])
{
	char ch, ep;
	int iters = 0;
	FILE *toimport = NULL;

	while ((ch = getopt(argc, argv, "f:i:?h")) != -1) 
		switch (ch) {
		case 'f':
			if ((toimport = fopen(optarg, "r")) == NULL)
				errx(EX_DATAERR, "Unable to open file %s", optarg);
			break;
		case 'i':
			if ((iters = (int)strtol(optarg, &ep, 10)) <= 0)
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

   Tsp *tsp = import_tsp(toimport);
	return EX_OK;
}

static void
usage(void)
{
	(void)fprintf(stderr, "usage tsp -f [filename] -i [number of iterations]\n");
	(void)fprintf(stderr, "-f [filename]              The filename from which \
the distances should be loaded.\n");
	(void)fprintf(stderr, "-i [number of iterations]  The number of iterations \
which should be used to compute the shortest path.\n");
	(void)fprintf(stderr, "\n");
	(void)fprintf(stderr, "Travelling salesman solver version %s.\n", VERSION);
	(void)fprintf(stderr, "Report bugs to %s.\n", PACKAGE_BUGREPORT);

	exit(EX_USAGE);
}
