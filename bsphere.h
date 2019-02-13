#ifndef BTG_BSPHERE_H
#define BTG_BSPHERE_H

#include <stdio.h>

#include "struct.h"

btg_bsphere *read_bsphere (FILE *f, btg_base *base, unsigned int ver, int index);
int write_bsphere (FILE *f, btg_bsphere *bsphere , unsigned int ver);
void free_bsphere (btg_bsphere *bsphere);

void remove_unused_bspheres (btg_bsphere *bsphere);
void check_same_bspheres (btg_bsphere *bsphere);

#endif