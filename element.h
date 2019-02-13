#ifndef BTG_ELEMENT_H
#define BTG_ELEMENT_H

#include <stdio.h>

#include "struct.h"

btg_element *read_element (FILE *f, btg_base *base, unsigned int ver, unsigned char type, unsigned char mask, char *material);
int write_element(FILE *f, btg_element *elem, unsigned int ver, unsigned char type, unsigned char mask, char *material);
void free_element(btg_element *elem, unsigned char type);

#endif