#ifndef IO_H
#define IO_H

#include "tsp.h"

Tsp* import_tsp(char* file);
void export_tsp(char* file, Tsp* tsp);

#endif
