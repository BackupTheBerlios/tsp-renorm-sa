#ifndef RENORMALIZATION_H
#define RENORMALIZATION_H

#define MARGE 0.1

#define EDGE_12 1
#define EDGE_1A 2
#define EDGE_14 4
#define EDGE_23 8
#define EDGE_2A 16
#define EDGE_2B 32
#define EDGE_3B 64
#define EDGE_35 128
#define EDGE_4A 256
#define EDGE_4C 512
#define EDGE_45 1024
#define EDGE_46 2048
#define EDGE_5B 4096
#define EDGE_5D 8192
#define EDGE_58 16384
#define EDGE_6C 32768
#define EDGE_67 65536
#define EDGE_68 131072
#define EDGE_7C 262144
#define EDGE_7D 524288
#define EDGE_8D 1048576
#define EDGE_AB 2097152
#define EDGE_AC 4194304
#define EDGE_AD 8388608
#define EDGE_BC 16777216
#define EDGE_BD 33554432
#define EDGE_CD 67108864

void renormalize(Tsp *tsp);

int getRoute(int cell_topleft, int cell_topright,
             int cell_bottomleft, int cell_bottomright,
             int route_part, int route);

#endif
