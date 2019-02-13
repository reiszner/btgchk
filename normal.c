#include <stdlib.h>

#include "normal.h"
#include "raw.h"

btg_normal *read_normal (FILE *f, btg_base *base, unsigned int ver, int index) {

	btg_normal *new = NULL;

	if (base == NULL) {
		fprintf(stderr, "pointer to base is NULL! break.\n");
		return NULL;
	}

	if ((new = malloc(sizeof(*new))) == NULL) {
		fprintf(stderr, "No memory left for normal! break.\n");
		return NULL;
	}
	new->next = NULL;
	new->valid = 1;
	new->index = index;
	new->count = 0;
	new->alias = NULL;

	base->normal_array[index] = new;

	if (read_uchar(f, &new->x)) printf("char Ooops\n");
	if (read_uchar(f, &new->y)) printf("char Ooops\n");
	if (read_uchar(f, &new->z)) printf("char Ooops\n");

	return new;
}

int write_normal (FILE *f, btg_normal *normal, unsigned int ver) {

	if (normal->valid) {
		if (write_uchar(f, &normal->x)) return 1;
		if (write_uchar(f, &normal->y)) return 2;
		if (write_uchar(f, &normal->z)) return 3;
	}

	return 0;
}

void free_normal (btg_normal *normal) {
	btg_normal *temp = NULL;
	while (normal) {
		temp = normal->next;
		free(normal);
		normal = temp;
	}
}



void remove_unused_normals (btg_normal *normal) {

	size_t cnt = 0, error = 0;

	while (normal) {
		if (normal->valid) {
			if (normal->count == 0) {
				normal->valid = 0;
				error++;
			}
			else normal->index = cnt++;
		}
		normal = normal->next;
	}

	if (error) printf("%zd unused normals deleted.\n", error);
}

void check_same_normals (btg_normal *normal) {

	int error = 0;
	btg_normal *temp;

	while (normal) {
		if (normal->valid) {
			temp = normal->next;
			while (temp && normal->valid) {
				if (
			    temp->valid &&
			    normal->x == temp->x &&
			    normal->y == temp->y &&
			    normal->z == temp->z
			    ) {
					if (normal->count > temp->count) {
						temp->alias = normal;
						normal->count += temp->count;
						temp->valid = 0;
					}
					else {
						normal->alias = temp;
						temp->count += normal->count;
						normal->valid = 0;
					}
					error++;
				}
				temp = temp->next;
			}
		}
		normal = normal->next;
	}

	if (error) printf("%d duplicated normals removed.\n", error);
}
