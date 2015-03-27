/*
 * LBFF: External objects, interacting with fluid
 *
 * Copyright (C) 2011 LBFF Authors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation: http://www.gnu.org/licenses/
 */
#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 * Generation of one force from the geometrical centre of object
 */
int EXTOBJ_ForceCalcSimple(void *_obj, const EXTOBJ_force_t * input_forces,
			   uint in_forces_num, EXTOBJ_force_t * output_forces)
{
	int i;
	lb_float maxx = 0, maxy = 0, maxz = 0, minx = 1e20, miny = 1e20, minz =
	    1e20;
	EXTOBJ_obj_p obj = (EXTOBJ_obj_p) _obj;

	output_forces[0].force = obj->turnedOn ? 0.01 : 0;

	for (i = 0; i < obj->points_cnt; ++i) {
		maxx = max(maxx, obj->points[i].x);
		minx = min(minx, obj->points[i].x);

		maxy = max(maxy, obj->points[i].y);
		miny = min(miny, obj->points[i].y);

		maxz = max(maxz, obj->points[i].z);
		minz = min(minz, obj->points[i].z);
	}
	output_forces[0].points.x = obj->pos.x + (maxx + minx) / 2;
	output_forces[0].points.y = obj->pos.y + (maxy + miny) / 2;
	output_forces[0].points.z = obj->pos.z + (maxz + minz) / 2;

	if (obj->turnedOn) {
		output_forces[0].vector.x = 0;
		output_forces[0].vector.y = 1 * output_forces[0].force;
		output_forces[0].vector.z = 0;
	} else {
		output_forces[0].vector.x = 0;
		output_forces[0].vector.y = 0;
		output_forces[0].vector.z = 0;
	}

	input_forces = input_forces;
	in_forces_num = in_forces_num;

	return 1;
}

EXTOBJ_obj_set_p EXTOBJ_CreateObjectSet(int capacity)
{
	EXTOBJ_obj_set_p obj_set =
	    (EXTOBJ_obj_set_p) malloc(sizeof(EXTOBJ_obj_set_t));

	obj_set->objects =
	    (EXTOBJ_obj_p) malloc(sizeof(EXTOBJ_obj_t) * capacity);
	obj_set->capacity = capacity;
	obj_set->count = 0;

	return obj_set;
}

/*
 * Object constructor
 */
void EXTOBJ_AddObject(EXTOBJ_obj_set_p obj_set, LB3D_p pos,
		      EXTOBJ_obj_type_t type)
{
	if (NULL != obj_set && obj_set) {
		EXTOBJ_obj_p obj = &(obj_set->objects[obj_set->count++]);

		switch (type) {
		case EXTOBJ_TYPE_SIMPLE:
			{
				LB3D_t points[] = {
					{0, 0, 0}
					,
					{10, 0, 0}
					,
					{10, 10, 0}
					,
					{0, 10, 0}
				};
				size_t points_cnt =
				    sizeof(points) / sizeof(LB3D_t);
				size_t size = sizeof(LB3D_t) * points_cnt;
				obj->points = (LB3D_p) malloc(size);
				memcpy(obj->points, points, size);
				memcpy(&(obj->pos), pos, sizeof(LB3D_t));

				obj->recalculate_force = EXTOBJ_ForceCalcSimple;
				obj->points_cnt = points_cnt;
				obj->turnedOn = 0;
			}
			break;

		default:
			break;
		}
	}
}
