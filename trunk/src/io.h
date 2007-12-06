#ifndef IO_H
#define IO_H

#include "tsp.h"

Tsp    *import_tsp(FILE * file);
void    export_tsp(FILE * stream, Tsp * tsp);

#endif
