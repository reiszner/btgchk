#ifndef BTG_OBJECT_H
#define BTG_OBJECT_H

#include <stdio.h>

#include "struct.h"

btg_object *read_object (FILE *f, btg_base *base, unsigned int ver);
int write_object (FILE *f, btg_object *obj, unsigned int ver);
void free_object (btg_object *obj);

#endif