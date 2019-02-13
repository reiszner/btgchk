#include <stdlib.h>

#include "raw.h"
#include "element.h"
#include "bsphere.h"
#include "vertex.h"
#include "normal.h"
#include "color.h"
#include "texcoo.h"
#include "point.h"
#include "triangle.h"
#include "geometry.h"

btg_element *read_element (FILE *f, btg_base *base, unsigned int ver, unsigned char type, unsigned char mask, char *material) {

	int i, num;
	btg_element  *elem    = NULL;
	btg_bsphere  *bsphere = NULL, *last_b = NULL;
	btg_vertex   *vertex  = NULL, *last_v = NULL;
	btg_normal   *normal  = NULL, *last_n = NULL;
	btg_color    *color   = NULL, *last_c = NULL;
	btg_texcoo   *texcoo  = NULL, *last_t = NULL;
	btg_geometry *geo     = NULL, *last_g = NULL;

	if ((elem = malloc(sizeof(*elem))) == NULL) {
		fprintf(stderr, "No memory left for element! break.\n");
		return NULL;
	}
	elem->next = NULL;
	elem->element = NULL;
	elem->valid = 1;
	elem->count = 0;

	if (read_uint(f, &elem->num_bytes)) {
		printf("problem while reading byte counter! break.\n");
		free_element(elem, type);
		return NULL;
	}

	num = 0;
	for (i = 0 ; i < 4 ; i++) {
		if (mask & (1 << i)) num++;
	}

	switch (type) {

		case OBJ_BS:
			if (elem->num_bytes % 28) {
				fprintf(stderr, "byte counter in bounding sphere isn't correct! break.\n");
				free_element(elem, type);
				return NULL;
			}
			elem->count = elem->num_bytes / 28;
			for (i = 0 ; i < elem->count ; i++) {
				if ((bsphere = read_bsphere(f, base, ver, i)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_b) {
					last_b->next = bsphere;
					last_b = last_b->next;
				}
				else {
					base->bsphere = elem->element = last_b = bsphere;
				}
			}
			printf("%d bounding sphere ...\n", i);
			break;

		case OBJ_VERTEX:
			if (elem->num_bytes % 12) {
				fprintf(stderr, "byte counter in vertex isn't correct! break.\n");
				free_element(elem, type);
				return NULL;
			}
			elem->count = elem->num_bytes / 12;
			if ((base->vertex_array = calloc(sizeof(*base->vertex_array), elem->count)) == NULL) {
				fprintf(stderr, "No memory left for vertex array! break.\n");
				free_element(elem, type);
				return NULL;
			}
			for (i = 0 ; i < elem->count ; i++) {
				if ((vertex = read_vertex(f, base, ver, i)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_v) {
					last_v->next = vertex;
					last_v = last_v->next;
				}
				else {
					base->vertex = elem->element = last_v = vertex;
				}
			}
			printf("%d vertex ...\n", i);
			break;

		case OBJ_NORMAL:
			if (elem->num_bytes % 3) {
				fprintf(stderr, "byte counter in normal isn't correct! break.\n");
				free_element(elem, type);
				return NULL;
			}
			elem->count = elem->num_bytes / 3;
			if ((base->normal_array = calloc(sizeof(*base->normal_array), elem->count)) == NULL) {
				fprintf(stderr, "No memory left for normal array! break.\n");
				free_element(elem, type);
				return NULL;
			}
			for (i = 0 ; i < elem->count ; i++) {
				if ((normal = read_normal(f, base, ver, i)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_n) {
					last_n->next = normal;
					last_n = last_n->next;
				}
				else {
					base->normal = elem->element = last_n = normal;
				}
			}
			printf("%d normal ...\n", i);
			break;

		case OBJ_COLOR:
			if (elem->num_bytes % 16) {
				fprintf(stderr, "byte counter in color isn't correct! exit.\n");
				free_element(elem, type);
				return NULL;
			}
			elem->count = elem->num_bytes / 16;
			if ((base->color_array = calloc(sizeof(*base->color_array), elem->count)) == NULL) {
				fprintf(stderr, "No memory left for color array! break.\n");
				free_element(elem, type);
				return NULL;
			}
			for (i = 0 ; i < elem->count ; i++) {
				if ((color = read_color(f, base, ver, i)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_c) {
					last_c->next = color;
					last_c = last_c->next;
				}
				else {
					base->color = elem->element = last_c = color;
				}
			}
			printf("%d color ...\n", i);
			break;

		case OBJ_TEXCOO:
			if (elem->num_bytes % 8) {
				fprintf(stderr, "byte counter in texture coordinate isn't correct! exit.\n");
				free_element(elem, type);
				return NULL;
			}
			elem->count = elem->num_bytes / 8;
			if ((base->texcoo_array = calloc(sizeof(*base->texcoo_array), elem->count)) == NULL) {
				fprintf(stderr, "No memory left for texcoo array! break.\n");
				free_element(elem, type);
				return NULL;
			}
			for (i = 0 ; i < elem->count ; i++) {
				if ((texcoo = read_texcoo(f, base, ver, i)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_t) {
					last_t->next = texcoo;
					last_t = last_t->next;
				}
				else {
					base->texcoo = elem->element = last_t = texcoo;
				}
			}
			printf("%d texture coordinate ...\n", i);
			break;

		case OBJ_POINTS:
		case OBJ_TRIS:
		case OBJ_STRIPE:
		case OBJ_FAN:
			if (ver == 7) {
				if (elem->num_bytes % (2 * num)) {
					fprintf(stderr, "byte counter in geomentry isn't correct! exit.\n");
					free_element(elem, type);
					return NULL;
				}
				elem->count = elem->num_bytes / (2 * num);
			}
			if (ver == 10) {
				if (elem->num_bytes % (4 * num)) {
					fprintf(stderr, "byte counter in geomentry isn't correct! exit.\n");
					free_element(elem, type);
					return NULL;
				}
				elem->count = elem->num_bytes / (4 * num);
			}
			for (i = 0 ; i < elem->count ; i++) {
				if ((geo = read_geometry(f, base, ver, mask)) == NULL) {
					free_element(elem, type);
					return NULL;
				}
				if (last_g) {
					last_g->next = geo;
					last_g = last_g->next;
				}
				else {
					elem->element = last_g = geo;
				}
			}
			break;

			default:
				fprintf(stderr, "unknown object type %d\n", type);
				break;
	}
	return elem;
}



int write_element (FILE *f, btg_element  *elem, unsigned int ver, unsigned char type, unsigned char mask, char *material) {

	int i = 0, num;
	btg_bsphere  *bsphere = NULL;
	btg_vertex   *vertex  = NULL;
	btg_normal   *normal  = NULL;
	btg_color    *color   = NULL;
	btg_texcoo   *texcoo  = NULL;
	btg_geometry *geo     = NULL;

	if (elem->valid) {
		switch (type) {
			case OBJ_BS:
				elem->count = 0;
				bsphere = elem->element;
				while (bsphere) {
					if (bsphere->valid) elem->count++;
					bsphere = bsphere->next;
				}
				elem->num_bytes = elem->count * 28;
				if (write_uint(f, &elem->num_bytes)) return 1;

				bsphere = elem->element;
				while (bsphere) {
					if (bsphere->valid) {
						if (write_bsphere(f, bsphere, ver)) return 2;
					}
					bsphere = bsphere->next;
				}
				break;

			case OBJ_VERTEX:
				elem->count = 0;
				vertex = elem->element;
				while (vertex) {
					if (vertex->valid) elem->count++;
					vertex = vertex->next;
				}
				elem->num_bytes = elem->count * 12;
				if (write_uint(f, &elem->num_bytes)) return 1;

				vertex = elem->element;
				while (vertex) {
					if (vertex->valid) {
						if (write_vertex(f, vertex, ver)) return 2;
					}
					vertex = vertex->next;
				}
				break;

			case OBJ_NORMAL:
				elem->count = 0;
				normal = elem->element;
				while (normal) {
					if (normal->valid) elem->count++;
					normal = normal->next;
				}
				elem->num_bytes = elem->count * 3;
				if (write_uint(f, &elem->num_bytes)) return 1;

				normal = elem->element;
				while (normal) {
					if (normal->valid) {
						if (write_normal(f, normal, ver)) return 2;
					}
					normal = normal->next;
				}
				break;

			case OBJ_COLOR:
				elem->count = 0;
				color = elem->element;
				while (color) {
					if (color->valid) elem->count++;
					color = color->next;
				}
				elem->num_bytes = elem->count * 16;
				if (write_uint(f, &elem->num_bytes)) return 1;

				color = elem->element;
				while (color) {
					if (color->valid) {
						if (write_color(f, color, ver)) return 2;
					}
					color = color->next;
				}
				break;

			case OBJ_TEXCOO:
				elem->count = 0;
				texcoo = elem->element;
				while (texcoo) {
					if (texcoo->valid) elem->count++;
					texcoo = texcoo->next;
				}
				elem->num_bytes = elem->count * 8;
				if (write_uint(f, &elem->num_bytes)) return 1;

				texcoo = elem->element;
				while (texcoo) {
					if (texcoo->valid) {
						if (write_texcoo(f, texcoo, ver)) return 2;
					}
					texcoo = texcoo->next;
				}
				break;

			case OBJ_POINTS:
			case OBJ_TRIS:
			case OBJ_STRIPE:
			case OBJ_FAN:
				num = 0;
				for (i = 0 ; i < 4 ; i++) {
					if (mask & (1 << i)) num++;
				}
				elem->count = 0;
				geo = elem->element;
				while (geo) {
					if (geo->valid) elem->count++;
					geo = geo->next;
				}
				if (ver == 7 ) elem->num_bytes = elem->count * (2 * num);
				if (ver == 10) elem->num_bytes = elem->count * (4 * num);
				if (write_uint(f, &elem->num_bytes)) return 1;

				geo = elem->element;
				while (geo) {
					if (geo->valid) {
						if (write_geometry(f, geo, ver, mask)) return 2;
					}
					geo = geo->next;
				}
				break;

			default:
				fprintf(stderr, "unknown object type %d\n", type);
				break;
		}
	}

	return 0;
}

void free_element(btg_element *elem, unsigned char type) {
	btg_element *temp = NULL;
	while (elem) {
		temp = elem->next;
		if (elem->element) {
			switch (type) {

				case OBJ_BS:
					free_bsphere(elem->element);
					break;

				case OBJ_VERTEX:
					free_vertex(elem->element);
					break;

				case OBJ_NORMAL:
					free_normal(elem->element);
					break;

				case OBJ_COLOR:
					free_color(elem->element);
					break;

				case OBJ_TEXCOO:
					free_texcoo(elem->element);
					break;

// TODO: free points and triangles

			}
		}
		free(elem);
		elem = temp;
	}
}
