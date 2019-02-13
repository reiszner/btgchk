#include <stdio.h>
#include <stdlib.h>

#include "raw.h"
#include "object.h"
#include "property.h"
#include "element.h"

btg_object *read_object (FILE *f, btg_base *base, unsigned int ver) {

	int i;
	unsigned int prop_cnt;
	unsigned short x;
	btg_object *obj = NULL;
	btg_element *elem = NULL, *last = NULL;

	if ((obj = malloc(sizeof(*obj))) == NULL) {
		fprintf(stderr, "No memory left for object! break.\n");
		return NULL;
	}
	obj->elem_cnt = 0;
	obj->prop_mask = 0;
	obj->prop_material = NULL;
	obj->elem_list = NULL;
	obj->next = NULL;

	if (read_uchar(f, &obj->obj_type)) printf("char Ooops\n");
	if (ver == 7) {
		if (read_ushort(f, &x)) printf("short Ooops\n");
		prop_cnt = x;
		if (read_ushort(f, &x)) printf("short Ooops\n");
		obj->elem_cnt = x;
	}
	else if (ver == 10) {
		if (read_uint(f, &prop_cnt)) printf("int Ooops\n");
		if (read_uint(f, &obj->elem_cnt)) printf("int Ooops\n");
	}

	for (i = 0 ; i < prop_cnt ; i++) {
		read_property (f, obj, ver);
	}

	for (i = 0 ; i < obj->elem_cnt ; i++) {
		elem = read_element (f, base, ver, obj->obj_type, obj->prop_mask, obj->prop_material);
		if (last) {
			last->next = elem;
			last = last->next;
		}
		else {
			obj->elem_list = last = elem;
		}
	}

	return obj;
}

int write_object (FILE *f, btg_object *obj, unsigned int ver) {

	unsigned short us;
	unsigned int pcnt;
	btg_element *elem = NULL;

	pcnt = 0;
	if (obj->prop_mask) pcnt++;
	if (obj->prop_material) pcnt++;

	obj->elem_cnt = 0;
	elem = obj->elem_list;
	while (elem) {
		if (elem->valid) obj->elem_cnt++;
		elem = elem->next;
	}

	if (write_uchar(f, &obj->obj_type)) {
		fprintf(stderr, "problem while writing object type! break.\n");
		return 1;
	}
	if (ver == 7) {
		us = pcnt;
		if (write_ushort(f, &us)) return 2;
		us = obj->elem_cnt;
		if (write_ushort(f, &us)) return 2;
	}
	else if (ver == 10) {
		if (write_uint(f, &pcnt)) return 3;
		if (write_uint(f, &obj->elem_cnt)) return 3;
	}
	if (pcnt) {
		write_property (f, obj, ver);
	}

	elem = obj->elem_list;
	while (elem) {
		if (elem->valid) {
			if (write_element (f, elem, ver, obj->obj_type, obj->prop_mask, obj->prop_material)) return 4;
		}
		elem = elem->next;
	}

	return 0;
}

void free_object (btg_object *obj) {
	btg_object *temp = NULL;
	if (obj) {
		while (obj) {
			temp = obj->next;
			if (obj->elem_cnt) free_element (obj->elem_list, obj->obj_type);
			free(obj);
			obj = temp;
		}
	}
}
