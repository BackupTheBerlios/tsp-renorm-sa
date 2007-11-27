#include <sysexits.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

Tsp* import_tsp(char* file)
{
    FILE* f;
    char buffer[128];
    char arg_string[64];
    int arg_int;
    double arg_double0, arg_double1;
    int header_finished;
    double x, y;
    Tsp* result;
    char* suc;
    
    //Reserve space for TSP
    result = calloc(1, sizeof(Tsp));
    if(!result) {
        fprintf(stderr, "Out of memory\n");
        exit(EX_OSERR);
    }
        
    //Open file for reading
    f = fopen(file, "r");    
    if(!f) {
        fprintf(stderr, "Unable to open file %s\n", file);
        exit(EX_DATAERR);
    }
    
    //Read header
    while(fgets(buffer, 128, f))
    {
        if(sscanf(buffer, "NAME : %32s", arg_string) == 1)
            strncpy(result->name, arg_string, 32);
        else if(sscanf(buffer, "COMMENT : %64s", arg_string) == 1)
            strncpy(result->comment, arg_string, 64);
        else if(sscanf(buffer, "TYPE : %32s", arg_string) == 1) {
            if(strcmp(arg_string, "TSP")) {
                fprintf(stderr, "Invalid input file\n");
                exit(EX_DATAERR);
            }
        }
        else if(sscanf(buffer, "DIMENSION : %d", &arg_int) == 1)
            result->dimension = arg_int;
        else if(sscanf(buffer, "EDGE_WEIGHT_TYPE : %32s", arg_string) == 1) {
            if(strcmp(arg_string, "EUC_2D") == 0)
                result->distance_type = EUC_2D;
            else {
                fprintf(stderr, "Format %s not yet supported\n", arg_string);
                exit(EX_DATAERR);
            }
        }
        else
            break;
    }
    
    //Check dimension
    if(result->dimension <= EX_OSERR) {
        fprintf(stderr, "No dimension specified, or invalid one\n");
        exit(EX_DATAERR);
    }

    //Read in cities
    result->cities = calloc(result->dimension, sizeof(City));
    result->tour = calloc(result->dimension, sizeof(int));
    if(!result->cities || !result->tour) {
        fprintf(stderr, "Out of memory\n");
        exit(EX_OSERR);
    }
    
    //Read data
    while(fgets(buffer, 128, f))
    {
        if(sscanf(buffer, "%d %f %f", &arg_int, &arg_double0, &arg_double1) == 3) {
            if(arg_int > result->dimension) {
                fprintf(stderr, "Incorrect city index\n");
                exit(EX_DATAERR);                
            }
            result->cities[arg_int - 1].x = arg_double0;
            result->cities[arg_int - 1].y = arg_double1;
        }
    }
    
    for(arg_int = 0; arg_int < result->dimension; arg_int++)
        result->tour[arg_int] = arg_int;
    
    return result;
}

void export_tsp(char* file, Tsp* tsp)
{
/*    FILE* f;
    City* city;
    
    if(!tour->numCities)
        return;
    
    f = fopen(filename, "w");
    
    if(!f) {
        fprintf(stderr, "Unable to open file\n");
        exit(1);
    }
    
    city = tour->cities;
    do {
        fprintf(f, "%s\t%f\t%f\n", city->name, city->x, city->y);
        city = city->next;
    } while(city != tour->cities);*/
}
