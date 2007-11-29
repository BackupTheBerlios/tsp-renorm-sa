#include <sysexits.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "io.h"

Tsp* 
import_tsp(FILE* file)
{
    char buffer[128];
    char arg_string[64];
    int arg_int;
    double arg_double0, arg_double1;
    int header_finished;
    double x, y;
    Tsp* result;
    char* suc;
    
    if ((result = calloc(1, sizeof(Tsp))) == NULL)
        errx(stderr, "Out of memory\n");

	 assert(file != NULL);
    
    /* 
	  * Read the file header. This should be in the following form:
	  *
	  * NAME: <file name, max 32 bytes> 
	  * COMMENT: <some comments, max 64 bytes>
	  * TYPE: <file type, only TSP is supported>
	  * DIMENSION: <number of cities>
	  * EDGE_WEIGHT_TYPE: <in which dimension are the edges, currently only
	  *	EUC_2D is supported>
	  */
    while (fgets(buffer, 128, file)) {
        if (sscanf(buffer, "NAME : %32s", arg_string) == 1)
            strncpy(result->name, arg_string, 32);
        else if (sscanf(buffer, "COMMENT : %64s", arg_string) == 1)
            strncpy(result->comment, arg_string, 64);
        else if (sscanf(buffer, "TYPE : %3s", arg_string) == 1)  {
            if (strcmp(arg_string, "TSP")) 
                errx(EX_DATAERR, "Invalid input file\n");
		  } else if (sscanf(buffer, "DIMENSION : %d", &arg_int) == 1)
            result->dimension = arg_int;
        else if (sscanf(buffer, "EDGE_WEIGHT_TYPE : %32s", arg_string) == 1) {
            if (strcmp(arg_string, "EUC_2D") == 0)
                result->distance_type = EUC_2D;
            else 
                errx(EX_DATAERR, "Format %s not yet supported", arg_string);
		  } else
            break;
    }

    /* Do some sanity check. */
    if (result->dimension <= 0) 
        errx(EX_DATAERR, "No dimension specified, or invalid one");

	 if (((result->cities = calloc(result->dimension, sizeof(City))) == NULL)
			 || ((result->tour = calloc(result->dimension, sizeof(int))) == NULL))
		 errx(EX_OSERR, "Out of memory");
    
	 /* Read the cities. */
    while (fgets(buffer, 128, file))
    {
        if (sscanf(buffer, "%d %lf %lf", &arg_int, &arg_double0, &arg_double1) == 3) {
            if (arg_int > result->dimension) 
                errx(EX_DATAERR, "Incorrect city index\n");
            
            result->cities[arg_int - 1].x = arg_double0;
            result->cities[arg_int - 1].y = arg_double1;
        }
    }
    
   for (arg_int = 0; arg_int < result->dimension; arg_int++)
        result->tour[arg_int] = arg_int;

	/* Center all the cities around the origin. */
	double x_max = -1;
	double y_max = -1;

	for (int i = 0; i < result->dimension; i++) {
		if (result->cities[i].x > x_max)
			x_max = result->cities[i].x;
		if (result->cities[i].y > y_max)
			y_max = result->cities[i].y;
	}

	for (int i = 0; i < result->dimension; i++) {
		result->cities[i].x -= x_max / 2;
		result->cities[i].y -= y_max / 2;
	}
    
   return result;
}

void 
export_tsp(FILE* stream, Tsp* tsp)
{
/*    FILE* f;
    City* city;
    
    if (!tour->numCities)
        return;
    
    f = fopen(filename, "w");
    
    if (!f) {
        fprintf(stderr, "Unable to open file\n");
        exit(1);
    }
    
    city = tour->cities;
    do {
        fprintf(f, "%s\t%f\t%f\n", city->name, city->x, city->y);
        city = city->next;
    } while(city != tour->cities);*/
}


