#include <stdlib.h>

#include "raw.h"
#include "header.h"
#include "object.h"
#include "vertex.h"
#include "normal.h"
#include "color.h"
#include "texcoo.h"
#include "point.h"
#include "triangle.h"
#include "edge.h"

int read_header (FILE *f, btg_header *head) {

	unsigned int cr;
	unsigned short cnt;

	if (head == NULL) {
		fprintf(stderr, "pointer to header is NULL! exit.\n");
		return 1;
	}

	printf("read Header ...\n");

	read_ushort(f, &head->version);
	if (head->version != 7 && head->version != 10) {
		fprintf(stderr, "unknown version (%d)! exit.\n", head->version);
		return 4;
	}
	printf("Version: %hd\n", head->version);

	read_ushort(f, &head->mag_num);
	if (head->mag_num != 0x5347) {
		fprintf(stderr, "Magic Number isn't 'SG'! exit.\n");
		return 5;
	}
	printf("Magic Number: %c%c\n", (head->mag_num >> 8), (head->mag_num & 0x00ff));

	read_uint(f, &cr);
	head->creation = cr;
	printf("Creation Time: %s", ctime(&head->creation));

	if (head->version == 7) {
		read_ushort(f, &cnt);
		head->num_object = cnt;
	}
	else if (head->version == 10) {
		read_uint(f, &head->num_object);
	}
	else {
		fprintf(stderr, "Unknown Version %hd! exit.\n", head->version);
		return 6;
	}

	if (head->num_object == 0) {
		fprintf(stderr, "Toplevel Objects is 0! exit.\n");
		return 7;
	}

	return 0;
}



int write_header (FILE *f, btg_header *head) {

	unsigned int cr;
	unsigned short cnt = 0;

	if (head == NULL) {
		fprintf(stderr, "pointer to header is NULL! exit.\n");
		return 1;
	}

	head->num_object = count_object(head->object);
	printf("write Header ...\n");
	printf("header has %d objects ...\n", head->num_object);

	if (write_ushort(f, &head->version)) return 1;
	if (write_ushort(f, &head->mag_num)) return 2;
	cr = head->creation;
	if (write_uint(f, &cr)) return 3;
	if (head->version == 7) {
		cnt = head->num_object;
		if (write_ushort(f, &cnt)) return 4;
	}
	else if (head->version == 10) {
		if (write_uint(f, &head->num_object)) return 4;
	}
	else {
		fprintf(stderr, "Unknown Version %hd! exit.\n", head->version);
		return 5;
	}

	return 0;
}


btg_header *new_header (btg_header **all_header) {

	btg_header *header = NULL, *temp = NULL;

	if (all_header == NULL) {
		fprintf(stderr, "double pointer to all_header is NULL! exit.\n");
		return header;
	}

	if ((header = malloc(sizeof (*header))) == NULL) {
		fprintf(stderr, "no memory left for header!\n");
		return header;
	}

	header->index = -1;
	header->airport[0] = '\0';
	header->creation = 0;
	header->version = 0;
	header->mag_num = 0;
	header->num_object = 0;
	header->base.min_x = 0.0;
	header->base.max_x = 0.0;
	header->base.min_y = 0.0;
	header->base.max_y = 0.0;
	header->base.min_lon = 0.0;
	header->base.max_lon = 0.0;
	header->base.min_lat = 0.0;
	header->base.max_lat = 0.0;
	header->base.holesize = 0.0;
	header->base.material = NULL;
	header->base.bsphere = NULL;
	header->base.vertex = NULL;
	header->base.vertex_array = NULL;
	header->base.normal = NULL;
	header->base.normal_array = NULL;
	header->base.texcoo = NULL;
	header->base.texcoo_array = NULL;
	header->base.color = NULL;
	header->base.color_array = NULL;
	header->base.point = NULL;
	header->base.point_last = NULL;
	header->base.triangle = NULL;
	header->base.triangle_last = NULL;
	header->base.edge = NULL;
	header->base.edge_last = NULL;
	header->base.fence = NULL;
	header->base.fence_last = NULL;
	header->object = NULL;
	header->runway = NULL;
	header->next = NULL;

	if (*all_header == NULL) {
		*all_header = header;
	}
	else {
		temp = *all_header;
		while (temp->next) temp = temp->next;
		temp->next = header;
	}

	return header;
}

void free_header (btg_header *head) {

	btg_header *temp;

	while (head) {
		temp = head->next;
		if (head) {
			if (head->object) free_object (head->object);
			free(head);
		}
		head = temp;
	}
}
