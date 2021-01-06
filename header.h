#ifndef BTG_HEADER_H
#define BTG_HEADER_H

#include <stdio.h>
#include <time.h>

#include "struct.h"

int read_header (FILE *f, btg_header *head);
int write_header (FILE *f, btg_header *head);
btg_header *new_header (btg_header **all_header);
void free_header (btg_header *head);

#endif