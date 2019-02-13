#ifndef BTG_VERTEX_H
#define BTG_VERTEX_H

#include <stdio.h>

#include "struct.h"

#define VERTEX_PRECITION 0.01

#define USE_ABSOLUTE 0
#define USE_RELATIVE 1
#define USE_PROJECTION 2

typedef struct vector_s {
	double x;
	double y;
	double z;
} vector;

btg_vertex *read_vertex (FILE *f, btg_base *base, unsigned int ver, int index);
int write_vertex (FILE *f, btg_vertex *vertex, unsigned int ver);
void free_vertex (btg_vertex *vertex);

void remove_unused_vertices (btg_vertex *vertex);
void check_same_vertices (btg_vertex *vertex);

double pydacoras (const btg_vertex *v0, const btg_vertex *v1, const short what);
void projection (const btg_bsphere *bsphere, btg_vertex *vertex);
vector get_vector (const btg_vertex *v0, const btg_vertex *v1, short proj);
double veclen (const vector vec);
vector vecunique (const vector vec0);
double vecproduct (const vector vec0, const vector vec1);
vector veccross (const vector vec0, const vector vec1);
double vecphi (const vector vec0, const vector vec1);
btg_vertex *intersection (const btg_vertex *v0, const btg_vertex *v1, const btg_vertex *v2, const btg_vertex *v3);
vector area_normal (const btg_vertex *v0, const btg_vertex *v1, const btg_vertex *v2);
#endif
