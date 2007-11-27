#ifndef TSP_H
#define TSP_H

typedef struct {
    double x;
    double y;
} City;


typedef struct {
    char name[32];
    char comment[64];
    
    int dimension;
    enum Distance_type {
        EUC_2D
    } distance_type;
    
    City *cities;
    int *tour;
} Tsp;

#endif
