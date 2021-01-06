/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2018 Sascha Reißner <reiszner@novaplan.at>
 * 
 * btg2dxf is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * btg2dxf is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <archive.h>
#include <archive_entry.h>

#include "raw.h"
#include "file.h"
#include "btgio.h"
#include "coord.h"
#include "check.h"
#include "edge.h"
#include "vertex.h"
#include "seam.h"

int main(int argc, char *argv[])
{

	FILE *infile = NULL, *outfile = NULL;
	char fullpath[PATH_MAX] = {"\0"}, basepath[PATH_MAX] = {"\0"}, filename[PATH_MAX] = {"\0"}, file[PATH_MAX] = {"\0"};
	char index_s[PATH_MAX] = {"\0"}, lat_s[PATH_MAX] = {"\0"}, lon_s[PATH_MAX] = {"\0"}, texture[64] = {"\0"}, rwy_als[PATH_MAX] = {"\0"};
	int opt, flagp = 0, flaga = 0, flagi = 0, flagl = 0, flagm = 0, flagn = 0, flagg = 0, index = 0, indices[10] = {0}, cnt;
	double holesize = -2.0;
	btg_header *all_header = NULL, *last_header = NULL, *temp = NULL;
	coord_cart cart;
	coord_geo geo;
	btg_vertex *v;

	while ((opt = getopt(argc, argv, "p:a:i:l:m:t:s:r:nh")) != -1) {
		switch (opt) {
			case 'p':
				strncpy(basepath, optarg, PATH_MAX);
				flagp = 1;
				break;
			case 'a':
				strncpy(filename, optarg, PATH_MAX);
				flaga = 1;
				break;
			case 'i':
				strncpy(index_s, optarg, PATH_MAX);
				index = atoi(index_s);
				flagi = 1;
				break;
			case 'l': /* latitude */
				strncpy(lat_s, optarg, PATH_MAX);
				flagl = 1;
				break;
			case 'm': /* meridian/longitude */
				strncpy(lon_s, optarg, PATH_MAX);
				flagm = 1;
				break;
			case 'n':
				flagn = 1;
				break;
			case 's':
				sscanf(optarg, "%lf", &holesize);
				break;
			case 't':
				strncpy(texture, optarg, 64);
				break;
			case 'r':
				strcat(rwy_als, optarg);
				strcat(rwy_als, "/");
				break;
			case 'h':
			default:
				fprintf(stderr, "Usage: %s -p basepath -a airport\n", argv[0]);
				fprintf(stderr, "       %s -p basepath -i tileindex\n", argv[0]);
				fprintf(stderr, "       %s -p basepath -l latitude -m longitude\n", argv[0]);
				fprintf(stderr, "additional parameters:\n");
				fprintf(stderr, "    -n                  check boundery to neighbour tiles\n");
				fprintf(stderr, "    -s meter            the maximum size (in meter) of holes to close\n");
				fprintf(stderr, "    -t texture          a texture for closing holes\n");
				fprintf(stderr, "    -r rwy[=als[@len]]  new approach light system for runway <rwy>\n");
				fprintf(stderr, "                          with layout <als> and a length of <len> meter\n");
				fprintf(stderr, "                          possible ALS layouts are:\n");
				fprintf(stderr, "                            ALSF-I  (default 3000 ft / 900 m)\n");
				fprintf(stderr, "                            ALSF-II (default 3000 ft / 900 m)\n");
				fprintf(stderr, "                            SSALR   (default 2400 ft / 720 m)\n");
				fprintf(stderr, "                            SSALF   (default 1400 ft / 420 m)\n");
				fprintf(stderr, "                            MALSF   (default 1400 ft / 420 m)\n");
				fprintf(stderr, "                            MALS    (default 1400 ft / 420 m)\n");
				return EXIT_FAILURE;
		}
	}

	if (!flagp) {
		fprintf(stderr, "Need a basepath! exit.\n");
		return EXIT_FAILURE;
	}

	if (flagl && !flagm) {
		fprintf(stderr, "need longitude addition to latitude! exit.\n");
		return EXIT_FAILURE;
	}
	if (flagm && !flagl) {
		fprintf(stderr, "need latitude addition to longitude! exit.\n");
		return EXIT_FAILURE;
	}
	if (flagl && flagm) flagg = 1;

	if (!flaga && !flagg && !flagi) {
		fprintf(stderr, "need a file or index or lat/lon! exit.\n");
		return EXIT_FAILURE;
	}
	if (flaga && flagg && !flagi) {
		fprintf(stderr, "need only one of file or lat/lon! exit.\n");
		return EXIT_FAILURE;
	}
	if (flaga && !flagg && flagi) {
		fprintf(stderr, "need only one of file or index! exit.\n");
		return EXIT_FAILURE;
	}
	if (!flaga && flagg && flagi) {
		fprintf(stderr, "need only one of index or lat/lon! exit.\n");
		return EXIT_FAILURE;
	}



	if (flaga) {

		runway_info *runway = NULL, *tmp = NULL;
		btg_bsphere *bs = NULL;

		get_airport_path (fullpath, basepath, filename);
		printf("Path: '%s'\n", fullpath);
		runway = get_airport_info (fullpath, rwy_als);

		tmp = runway;
		cnt = 0;
		geo.lon = 0.0;
		geo.lat = 0.0;
		geo.msl = 0.0;

		while (tmp) {
			geo.lon += tmp->threshold[0].lon + tmp->threshold[1].lon;
			geo.lat += tmp->threshold[0].lat + tmp->threshold[1].lat;
			cnt += 2;

			printf("Runway info:\n");
			printf("    Threshold 1:\n");
			printf("        Longitude   : %f\n", tmp->threshold[0].lon);
			printf("        Latitude    : %f\n", tmp->threshold[0].lat);
			printf("        Heading     : %f\n", tmp->threshold[0].heading);
			printf("        Displacement: %f\n", tmp->threshold[0].displacement);
			printf("        Stopway     : %f\n", tmp->threshold[0].stopw);
			printf("        Runway      : %02d", tmp->threshold[0].rwy_num);
			if (tmp->threshold[0].rwy_ord) {
				if      (tmp->threshold[0].rwy_ord == 1) printf("L");
				else if (tmp->threshold[0].rwy_ord == 2) printf("C");
				else if (tmp->threshold[0].rwy_ord == 3) printf("R");
			}
			printf("\n");
			if (tmp->threshold[0].als_layout != ALS_NOOP) {
				printf("        ALS change  : %s @ %d\n",
				    tmp->threshold[0].als_layout == ALS_ALSF1 ? "ALSF-I" :
				    tmp->threshold[0].als_layout == ALS_ALSF2 ? "ALSF-II" :
				    tmp->threshold[0].als_layout == ALS_SSALR ? "SSALR" :
				    tmp->threshold[0].als_layout == ALS_SSALF ? "SSALF" :
				    tmp->threshold[0].als_layout == ALS_MALSF ? "MALSF" :
				    tmp->threshold[0].als_layout == ALS_MALS ? "MALS" :
				    "delete",
				    tmp->threshold[0].als_len);
			}

			printf("    Threshold 2:\n");
			printf("        Longitude   : %f\n", tmp->threshold[1].lon);
			printf("        Latitude    : %f\n", tmp->threshold[1].lat);
			printf("        Heading     : %f\n", tmp->threshold[1].heading);
			printf("        Displacement: %f\n", tmp->threshold[1].displacement);
			printf("        Stopway     : %f\n", tmp->threshold[1].stopw);
			printf("        Runway      : %02d", tmp->threshold[1].rwy_num);
			if (tmp->threshold[1].rwy_ord) {
				if      (tmp->threshold[1].rwy_ord == 1) printf("L");
				else if (tmp->threshold[1].rwy_ord == 2) printf("C");
				else if (tmp->threshold[1].rwy_ord == 3) printf("R");
			}
			printf("\n");
			if (tmp->threshold[1].als_layout != ALS_NOOP) {
				printf("        ALS change  : %s @ %d\n",
				    tmp->threshold[1].als_layout == ALS_ALSF1 ? "ALSF-I" :
				    tmp->threshold[1].als_layout == ALS_ALSF2 ? "ALSF-II" :
				    tmp->threshold[1].als_layout == ALS_SSALR ? "SSALR" :
				    tmp->threshold[1].als_layout == ALS_SSALF ? "SSALF" :
				    tmp->threshold[1].als_layout == ALS_MALSF ? "MALSF" :
				    tmp->threshold[1].als_layout == ALS_MALS ? "MALS" :
				    "delete",
				    tmp->threshold[1].als_len);
			}

			tmp = tmp->next;
		}

// calculate the index for full path
		geo.lon = ((geo.lon / cnt) * M_PI) / 180.0;
		geo.lat = ((geo.lat / cnt) * M_PI) / 180.0;
		index = geo2index(geo);
		get_terrain_path (fullpath, basepath, index);

		snprintf(file, PATH_MAX, "%.4082s/%.8s", fullpath, filename);
		btg_decompress (file);

		snprintf(file, PATH_MAX, "%.4082s/%.8s.btg", fullpath, filename);
		if ((infile = fopen(file, "rb")) == NULL) {
			fprintf(stderr, "File '%s' doesn't exist! exit.\n", file);
			return EXIT_FAILURE;
		}

		if ((last_header = new_header (&all_header)) == NULL) {
			fprintf(stderr, "no memory left for header!\n");
			return EXIT_FAILURE;
		}

		if (read_btg (infile, last_header)) {
			fprintf(stderr, "Problem while reading btg-file '%s'! exit.\n", file);
			fclose (infile);
			return EXIT_FAILURE;
		}
		fclose (infile);
		strncpy(last_header->airport, filename, 8);

		if (holesize > -1.0)
			last_header->base.holesize = holesize;
		else
			last_header->base.holesize = 350.0;
		if (strlen(texture))
			last_header->base.material = texture;
		else
			last_header->base.material = NULL;
		last_header->runway = runway;

// correct index from bounding sphere
		bs = last_header->base.bsphere;
		printf("*****\nBounding Sphere is on\nx: %f y: %f z: %f\n", bs->coord.x, bs->coord.y, bs->coord.z);
		cart.x = bs->coord.x;
		cart.y = bs->coord.y;
		cart.z = bs->coord.z;
		geo = cart2geo (cart);
		printf("Lon: %Lf Lat: %Lf MSL: %Lf\n*****\n", (geo.lon * 180.0) / M_PI, (geo.lat * 180.0) / M_PI, geo.msl);
		index = 0;
//		printf("a-flag - index: %d...\n", index);
	}

	if (flagi) {
		if (index <= 0) {
			fprintf(stderr, "invalid index '%s'! exit.\n", index_s);
			return EXIT_FAILURE;
		}
		printf("i-flag - index: %d...\n", index);
	}

	if (flagg) {
		coord_geo geo;
		geo.lon = (atof(lon_s) * M_PI) / 180.0;
		geo.lat = (atof(lat_s) * M_PI) / 180.0;
		geo.msl = 0.0;
		index = geo2index(geo);
//		printf("g-flag - index: %d...\n", index);
	}
	indices[0] = index;

	// find neighbours
	if (flagn) {
		if (last_header && last_header->index < 0 && strlen(last_header->airport) > 0) {
			find_maxima(indices, last_header);
		}
		else {
			find_neighbours(indices);
		}
	}

	printf("read in affected tiles ...\n");
	for (cnt = 0 ; cnt < 10 ; cnt++) {
		if (indices[cnt] != 0) {
			printf("read tile %d\n", indices[cnt]);

// we have now a index, read the tile in
			get_terrain_path (fullpath, basepath, indices[cnt]);
			snprintf(file, PATH_MAX, "%.4084s/%d", fullpath, indices[cnt]);
			btg_decompress (file);

			snprintf(file, PATH_MAX, "%.4084s/%d.btg", fullpath, indices[cnt]);
			if ((infile = fopen(file, "rb")) == NULL) {
				fprintf(stderr, "File '%s' doesn't exist! exit.\n", file);
			}
			else {

				if ((last_header = new_header (&all_header)) == NULL) {
					fprintf(stderr, "no memory left for header!\n");
					return EXIT_FAILURE;
				}

				if (read_btg (infile, last_header)) {
					fprintf(stderr, "Problem while reading btg-file '%s'! exit.\n", file);
				}
				fclose (infile);
				last_header->index = indices[cnt];

				if (holesize > -1.0)
					last_header->base.holesize = holesize;
				else
					last_header->base.holesize = 350.0;
				if (strlen(texture))
					last_header->base.material = texture;
				else
					last_header->base.material = NULL;
			}
		}
	}

	printf("mark 1...\n");
	printf("pointer: %p...\n", all_header);

// build projection
	temp = all_header;
	while (temp) {
		v = temp->base.vertex;
		while (v) {
			projection(all_header->base.bsphere, v);
			v = v->next;
		}
		temp = temp->next;
	}

	printf("mark 2...\n");
	printf("pointer: %p...\n", all_header);

// check btg datas
	temp = all_header;
	while (temp) {
		 check (temp);
		temp = temp->next;
	}

	printf("mark 3...\n");
	printf("pointer: %p...\n", all_header);
	printf("index  : %d...\n", all_header->index);
	printf("Name   : '%s'...\n", all_header->airport);

// if we have a airport, check seams between airport and tiles
	if (all_header->index < 0 && strlen(all_header->airport) > 0) {
		temp = all_header->next;
		while (temp) {
			printf("check seam ...\n");
			check_seam (temp, all_header->base.fence);
			check_seam_edges (temp, all_header->base.fence);
			temp = temp->next;
		}
	}

	printf("mark 4...\n");

	temp = all_header;
	while (temp) {

		if (temp->index < 0 && strlen(temp->airport) > 0) {
			snprintf(file, PATH_MAX, "%.4078s/%.8s-new.btg", fullpath, temp->airport);
		}
		else if (temp->index > 0 && strlen(temp->airport) == 0) {
			get_terrain_path (fullpath, basepath, temp->index);
			snprintf(file, PATH_MAX, "%.4084s/%d-new.btg", fullpath, temp->index);
		}
		else {
			fprintf(stderr, "Unknown header! Index: %d / Airport '%s'! exit.\n", temp->index, temp->airport);
			return EXIT_FAILURE;
		}

		if ((outfile = fopen(file, "wb")) == NULL) {
			fprintf(stderr, "Can't create file '%s'! exit.\n", file);
			return 2;
		}
		printf("write file '%s' ...\n", file);
		if (write_btg (outfile, temp)) {
			fprintf(stderr, "Problem while writing btg-file '%s'! exit.\n", file);
			fclose (outfile);
			return EXIT_FAILURE;
		}
		fclose (outfile);
		temp = temp->next;
	}

	printf("mark 5...\n");

	free_header(all_header);

	return EXIT_SUCCESS;
}
