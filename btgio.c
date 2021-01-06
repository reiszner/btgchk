#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "raw.h"
#include "btgio.h"
#include "point.h"
#include "triangle.h"
#include "object.h"

int read_btg (FILE *f, btg_header *header) {

	int i;
	btg_header *h;
	btg_object *obj = NULL, *last = NULL;

	if (header == NULL) {
		fprintf(stderr, "read_btg: pointer to header is NULL! exit.\n");
		return 1;
	}

	if (read_header(f, header)) {
		fprintf(stderr, "read_btg: Problem while reading Header! exit.\n");
		return 2;
	}
	h = header;

	for (i = 0 ; i < h->num_object ; i++) {
		printf("read_btg: read object %d ...\n", i);
		if ((obj = read_object (f, &h->base, h->version)) == NULL) {
			fprintf(stderr, "read_btg: problem while reading objects!\n");
			return 4;
		}
		if (last) {
			last->next = obj;
			last = last->next;
		}
		else {
			h->object = last = obj;
		}
	}
	printf("read_btg: file end on position: %ld\n", ftell(f));

	if (h->base.vertex_array) {
		free (h->base.vertex_array);
		h->base.vertex_array = NULL;
	}
	if (h->base.normal_array) {
		free (h->base.normal_array);
		h->base.normal_array = NULL;
	}
		if (h->base.color_array) {
		free (h->base.color_array);
		h->base.color_array = NULL;
	}
		if (h->base.texcoo_array) {
		free (h->base.texcoo_array);
		h->base.texcoo_array = NULL;
	}
/*
	obj = h->object;
	while (obj) {
		if (obj->obj_type == OBJ_POINTS) {
			if (add_point (obj->elem_list, &h->base, obj->prop_material)) {
				fprintf(stderr, "problem while add points! break.\n");
				return 5;
			}
		}
		if (obj->obj_type == OBJ_TRIS) {
			if (add_triangle (obj->elem_list, &h->base, obj->prop_material)) {
				fprintf(stderr, "problem while add triangles! break.\n");
				return 5;
			}
		}
		obj = obj->next;
	}
*/
	return 0;
}



int write_btg (FILE *f, btg_header *header) {

	if (header == NULL) {
		fprintf(stderr, "pointer to header is NULL! exit.\n");
		return 1;
	}

	if (write_header (f, header)) {
		fprintf(stderr, "Problem while writing header! exit.\n");
		return 2;
	}

	if (write_object (f, header->object, header->version)) {
		fprintf(stderr, "problem while writing objects!\n");
		return 3;
	}

	return 0;
}
