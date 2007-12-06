#include <math.h>
#include <err.h>
#include <stdlib.h>
#include <sysexits.h>
#include <assert.h>
#include <stdio.h>

#include "block.h"
#include "tsp.h"

double extern rotation = 0;

/* This contains all the cached values. */
static double _rotation = FP_NAN;
static double _x_max;
static double _x_min;
static double _y_max;
static double _y_min;
static City *_rot_cities = NULL;
static unsigned int _num_cities;

static void check_limits(void);
static void rotate(void);

grd    *
create_grd(const unsigned int *length, const unsigned int *height)
{
   /*
    * Update the cache containing the rotated cities if necessary. 
    */
   rotate();

   /*
    * Create a new grid and initialize all the values. 
    */
   grd    *new_grd;
   int    *indices;
   int     filled_blocks = 0;

   if ((new_grd = calloc(1, sizeof(grd))) == NULL)
      errx(EX_OSERR, "Not enough memory!");

   new_grd->length = *length;
   new_grd->height = *height;

   /*
    * Search how many blocks will have to be made. 
    */
   if ((indices = calloc(*length * *height, sizeof(int))) == NULL)
      errx(EX_OSERR, "Not enough memory!");

   for (int i = 0; i < (*length * *height); i++)
      indices[i] = INIT_INDEX;

   /*
    * Compute the necessary values which will be used to index the cities. 
    */
   double  x_step = fabs(_x_min - _x_max) / (double) (*length);
   double  y_step = fabs(_y_min - _y_max) / (double) (*height);

   for (int i = 0; i < _num_cities; i++) {
      unsigned int x = rint(floor((_rot_cities[i].x - _x_min) / x_step));
      unsigned int y = rint(floor((_rot_cities[i].y - _y_min) / y_step));
      /*
       * Search if the block is already counted for. 
       */
      int     j = 0;
      for (j = 0; j < (*length * *height); j++)
         if (indices[j] == ((x * *height) + y))
            break;
         else if (indices[j] == INIT_INDEX)
            break;

      if (indices[j] == INIT_INDEX) {
         /*
          * One more block will have to be allocated. 
          */
         filled_blocks++;
         indices[j] = ((x * *height) + y);
      }
   }

   /*
    * There should be one more box in the array. 
    */
   filled_blocks++;
   new_grd->filled_blocks = filled_blocks;

   if ((new_grd->block_cty = calloc(filled_blocks, sizeof(int))) == NULL)
      errx(EX_OSERR, "Not enough memory!");
   if ((new_grd->block_idx = calloc(filled_blocks, sizeof(int))) == NULL)
      errx(EX_OSERR, "Not enough memory!");

   for (int i = 0; i < filled_blocks; i++)
      new_grd->block_idx[i] = INIT_INDEX;

   for (int i = 0; i < _num_cities; i++) {
      unsigned int x = rint(floor((_rot_cities[i].x - _x_min) / x_step));
      unsigned int y = rint(floor((_rot_cities[i].y - _y_min) / y_step));
      /*
       * Search if the block has already an entry. 
       */
      int     j = 0;
      for (j = 0; j < filled_blocks; j++)
         if (new_grd->block_idx[j] == ((x * *height) + y)) {
            break;
         } else if (new_grd->block_idx[j] == INIT_INDEX) {
            break;
         }

      if (new_grd->block_idx[j] == INIT_INDEX) {
         /*
          * The index has to be stored. 
          */
         new_grd->block_idx[j] = ((x * *height) + y);
         new_grd->block_cty[j] = i;
      } else if (new_grd->block_idx[j] >= 0) {
         assert(new_grd->block_idx[j] == ((x * *height) + y));
         new_grd->block_cty[j] = MANY_CITIES;
      }
   }

   return new_grd;
}

int
has_city(grd * grid, int x, int y)
{
   assert(grid != NULL);
   assert(x < grid->height);
   assert(y < grid->length);

   for (int i = 0; i < grid->filled_blocks; i++)
      if (grid->block_idx[i] == ((x * grid->height) + y))
         return grid->block_cty[i];

   return NO_CITY;
}

void
print_cities(FILE * f)
{
   assert(f != NULL);
   (void) fprintf(f, "city_x city_y\n");
   for (int i = 0; i < _num_cities; i++)
      (void) fprintf(f, "%lf %lf\n", _rot_cities[i].x, _rot_cities[i].y);
}

void
free_grd(grd * grid)
{
   assert(grid != NULL);

   free(grid->block_cty);
   free(grid->block_idx);

   free(grid);
   grid = NULL;
}

static void
rotate(void)
{
   /*
    * It the old rotation is the same nothing has to be done. 
    */
   if (_rotation != FP_NAN && _rotation == rotation)
      return;

   /*
    * Free and allocate memory for the cities in the rotated plane. 
    */
   if (_rot_cities != NULL)
      free(_rot_cities);

   if ((_rot_cities = calloc(tsp->dimension, sizeof(City))) == NULL)
      errx(EX_OSERR, "No memory left!");

   /*
    * Rotate the cities. 
    */
   for (int i = 0; i < tsp->dimension; i++) {
      _rot_cities[i].x = tsp->cities[i].x * cos(rotation) +
          tsp->cities[i].y * sin(rotation);
      _rot_cities[i].y = -tsp->cities[i].x * sin(rotation) +
          tsp->cities[i].y * cos(rotation);
   }
   /*
    * Update the cached cities. 
    */
   _rotation = rotation;
   _num_cities = tsp->dimension;

   check_limits();
}

void
print_grd_lines(grd * grid, FILE * f)
{
   assert(grid != NULL);
   assert(f != NULL);
   /*
    * Compute the necessary values which will be used to index the cities. 
    */
   double  x_step = fabs(_x_min - _x_max) / (double) grid->length;
   double  y_step = fabs(_y_min - _y_max) / (double) grid->height;

   /*
    * Print the grid. At the end of each line a NA is needed to halt the line.
    * * See for more information on this the R manual for lines(). 
    */
   /*
    * The header. 
    */
   (void) fprintf(f, "x y\n");
   /*
    * First the vertical lines. 
    */
   for (double x = _x_min; x < _x_max; x += x_step) {
      (void) fprintf(f, "%lf %lf\n", x, _y_min);
      (void) fprintf(f, "%lf %lf\n", x, _y_max);
      (void) fprintf(f, "%lf NA\n");
   }
   (void) fprintf(f, "%lf %lf\n", _x_max, _y_min);
   (void) fprintf(f, "%lf %lf\n", _x_max, _y_max);
   (void) fprintf(f, "%lf NA\n");

   /*
    * The horizontal lines. 
    */
   for (double y = _y_min; y < _y_max; y += y_step) {
      (void) fprintf(f, "%lf %lf\n", _x_min, y);
      (void) fprintf(f, "%lf %lf\n", _x_max, y);
      (void) fprintf(f, "%lf NA\n");
   }
   (void) fprintf(f, "%lf %lf\n", _x_min, _y_max);
   (void) fprintf(f, "%lf %lf\n", _x_max, _y_max);
   (void) fprintf(f, "%lf NA\n");
}

void
print_grd_points(grd * grid, FILE * f)
{
   assert(grid != NULL);
   assert(f != NULL);
   /*
    * Compute the necessary values which will be used to index the cities. 
    */
   double  x_step = fabs(_x_min - _x_max) / (double) (grid->length);
   double  y_step = fabs(_y_min - _y_max) / (double) (grid->height);

   /*
    * Print the header. 
    */
   (void) fprintf(f, "x y pch\n");

   for (int x = 0; x < grid->length; x++) {
      for (int y = 0; y < grid->height; y++) {
         /*
          * Print the coordinate of the center of a box. 
          */
         (void) fprintf(f, "%lf %lf",
                        x * x_step + (_x_min + 0.5 * x_step),
                        y * y_step + (_y_min + 0.5 * y_step));
         /*
          * Determine the logo which will represent the center of the box. 
          * * These numbers are R codes. 
          */
         switch (has_city(grid, x, y)) {
         case NO_CITY:
            /*
             * pch=23 is a diamond. 
             */
            (void) fprintf(f, " 23\n");
            break;
         case MANY_CITIES:
            /*
             * pch=24 is a triangle point up. 
             */
            (void) fprintf(f, " 24\n");
            break;
         default:
            /*
             * pch=19 is a solid circle. 
             */
            (void) fprintf(f, " 19\n");
         }
      }
   }
}

static void
check_limits(void)
{
   if (_rot_cities == NULL)
      return;

   /*
    * Cache the x-max and min and the y-max and min. 
    */
   _x_max = -FP_INFINITE;
   _x_min = FP_INFINITE;
   _y_max = -FP_INFINITE;
   _y_min = FP_INFINITE;

   for (int i = 0; i < _num_cities; i++) {
      if (_rot_cities[i].x > _x_max)
         _x_max = _rot_cities[i].x;
      if (_rot_cities[i].x < _x_min)
         _x_min = _rot_cities[i].x;
      if (_rot_cities[i].y > _y_max)
         _y_max = _rot_cities[i].y;
      if (_rot_cities[i].y < _y_min)
         _y_min = _rot_cities[i].y;
   }

   /*
    * Built some margin to be sure that all the cities are included in 
    * * a box. 
    */
   _x_max += X_MARGIN;
   _x_min -= X_MARGIN;
   _y_max += Y_MARGIN;
   _y_min -= Y_MARGIN;
}
