#include <stdio.h>
#include <assert.h>

#include "path.h"
#include "tsp.h"

void
print_path(unsigned int *path, FILE * f)
{
   assert(path != NULL);
   assert(f != NULL);

   (void) sprintf(f, "x y\n");
   for (int i = 0; i < tsp->dimension; i++)
      (void) sprintf(f, "%lf %lf\n", tsp->cities[path[i]].x,
                     tsp->cities[path[i]].y);
}
