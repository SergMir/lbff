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
int EXTOBJ_ForceCalcSimple(void *_obj, EXTOBJ_force_t *input_forces, uint in_forces_num,
        EXTOBJ_force_t *output_forces)
{
  int i, maxx = 0, maxy = 0, maxz = 0, minx = 0x7FFFFFFF, miny = 0x7FFFFFFF, minz = 0x7FFFFFFF;
  EXTOBJ_obj_p obj = (EXTOBJ_obj_p)_obj;

  output_forces[0].force = obj->turnedOn ? 1 : 0;

  for (i = 0; i < obj->points_cnt; ++i)
  {
    maxx = max(maxx, obj->points[i * 3 + 0]);
    minx = min(minx, obj->points[i * 3 + 0]);

    maxy = max(maxy, obj->points[i * 3 + 1]);
    miny = min(miny, obj->points[i * 3 + 1]);

    maxz = max(maxz, obj->points[i * 3 + 2]);
    minz = min(minz, obj->points[i * 3 + 2]);
  }
  output_forces[0].points[0] = (maxx + minx) / 2;
  output_forces[0].points[1] = (maxy + miny) / 2;
  output_forces[0].points[2] = (maxz + minz) / 2;

  output_forces[0].vector[0] = 1;
  output_forces[0].vector[1] = 0;
  output_forces[0].vector[2] = 0;

  input_forces = input_forces;
  in_forces_num = in_forces_num;

  return 1;
}

/*
 * Object constructor
 */
EXTOBJ_obj_p EXTOBJ_CreateObject(EXTOBJ_obj_type_t type)
{
  EXTOBJ_obj_p obj = malloc(sizeof(EXTOBJ_obj_t));

  switch (type)
  {
  case EXTOBJ_TYPE_SIMPLE:
  {
    double points[4][3] = {{10, 10, 0}, {30, 10, 0}, {30, 30, 0}, {10, 30, 0}};
    size_t size = sizeof(double) * 3 * 4;
    obj->points = malloc(size);
    memcpy(obj->points, points, size);

    obj->recalculate_force = EXTOBJ_ForceCalcSimple;
    obj->points_cnt = 4;
    obj->turnedOn = 0;
  }
    break;

  default:
    break;
  }

  return obj;
}
