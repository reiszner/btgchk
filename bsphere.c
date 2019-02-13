#include <stdlib.h>

#include "bsphere.h"
#include "coord.h"
#include "raw.h"

btg_bsphere *read_bsphere (FILE *f, btg_base *base, unsigned int ver, int index) {

	btg_bsphere *new = NULL;
	coord_geo geo;
	coord_cart cart;

	if (base == NULL) {
		fprintf(stderr, "pointer to base is NULL! break.\n");
		return NULL;
	}

	if ((new = malloc(sizeof(*new))) == NULL) {
		fprintf(stderr, "No memory left for bounding sphere! break.\n");
		return NULL;
	}
	new->next = NULL;
	new->valid = 1;
	new->index = index;
	new->count = 1;

	if (read_double(f, &new->coord.x)) printf("double Ooops\n");
	if (read_double(f, &new->coord.y)) printf("double Ooops\n");
	if (read_double(f, &new->coord.z)) printf("double Ooops\n");
	if (read_float (f, &new->r)) printf("float Ooops\n");

	cart.x = new->coord.x;
	cart.y = new->coord.y;
	cart.z = new->coord.z;
	geo = cart2geo (cart);
	new->lat = geo.lat;
	new->lon = geo.lon;

	return new;
}

int write_bsphere (FILE *f, btg_bsphere *bsphere, unsigned int ver) {

	if (bsphere->valid) {
		if (write_double(f, &bsphere->coord.x)) return 1;
		if (write_double(f, &bsphere->coord.y)) return 2;
		if (write_double(f, &bsphere->coord.z)) return 3;
		if (write_float (f, &bsphere->r)) return 4;
	}

	return 0;
}

void free_bsphere (btg_bsphere *bsphere) {
	btg_bsphere *temp = NULL;
	while (bsphere) {
		temp = bsphere->next;
		free(bsphere);
		bsphere = temp;
	}
}



void remove_unused_bspheres (btg_bsphere *bsphere) {

	size_t cnt = 0, error = 0;

	while (bsphere) {
		if (bsphere->valid) {
			if (bsphere->count == 0) {
				bsphere->valid = 0;
				error++;
			}
			else bsphere->index = cnt++;
		}
		bsphere = bsphere->next;
	}

	if (error) printf("%zd unused bounding spheres deleted.\n", error);
}

void check_same_bspheres (btg_bsphere *bsphere) {

	int error = 0;
	btg_bsphere *temp;

	while (bsphere) {
		if (bsphere->valid) {
			temp = bsphere->next;
			while (temp && bsphere->valid) {
				if (
			    temp->valid &&
			    bsphere->coord.x == temp->coord.x &&
			    bsphere->coord.y == temp->coord.y &&
			    bsphere->coord.z == temp->coord.z &&
			    bsphere->r == temp->r
			    ) {
					if (bsphere->count > temp->count) {
						temp->valid = 0;
					}
					else {
						bsphere->valid = 0;
					}
					error++;
				}
				temp = temp->next;
			}
		}
		bsphere = bsphere->next;
	}

	if (error) printf("%d duplicated bounding spheres removed.\n", error);
}
