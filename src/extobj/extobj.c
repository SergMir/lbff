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
int EXTOBJ_ForceCalcSimple(void *_obj, const EXTOBJ_force_t *input_forces, uint in_forces_num,
        EXTOBJ_force_t *output_forces)
{
  int i;
  double maxx = 0, maxy = 0, maxz = 0, minx = 1e20, miny = 1e20, minz = 1e20;
  EXTOBJ_obj_p obj = (EXTOBJ_obj_p)_obj;

  output_forces[0].force = obj->turnedOn ? 1.0 : 0;

  for (i = 0; i < obj->points_cnt; ++i)
  {
    maxx = max(maxx, obj->points[i].x);
    minx = min(minx, obj->points[i].x);

    maxy = max(maxy, obj->points[i].y);
    miny = min(miny, obj->points[i].y);

    maxz = max(maxz, obj->points[i].z);
    minz = min(minz, obj->points[i].z);
  }
  output_forces[0].points.x = (maxx + minx) / 2;
  output_forces[0].points.y = (maxy + miny) / 2;
  output_forces[0].points.z = (maxz + minz) / 2;

  if (obj->turnedOn)
  {
    output_forces[0].vector.x = 0;
    output_forces[0].vector.y = 1 * output_forces[0].force;
    output_forces[0].vector.z = 0;
  }
  else
  {
    output_forces[0].vector.x = 0;
    output_forces[0].vector.y = 0;
    output_forces[0].vector.z = 0;
  }

  input_forces = input_forces;
  in_forces_num = in_forces_num;

  return 1;
}

/*
 * Object constructor
 */
EXTOBJ_obj_p EXTOBJ_CreateObject(EXTOBJ_obj_type_t type)
{
  EXTOBJ_obj_p obj = (EXTOBJ_obj_p)malloc(sizeof(EXTOBJ_obj_t));

  switch (type)
  {
  case EXTOBJ_TYPE_SIMPLE:
  {
    LB3D_t points[] = {{10, 10, 0}, {30, 10, 0}, {30, 30, 0}, {10, 30, 0}};
    size_t points_cnt = sizeof(points) / sizeof(LB3D_t);
    size_t size = sizeof(LB3D_t) * points_cnt;
    obj->points = (LB3D_p)malloc(size);
    memcpy(obj->points, points, size);

    obj->recalculate_force = EXTOBJ_ForceCalcSimple;
    obj->points_cnt = points_cnt;
    obj->turnedOn = 0;
  }
    break;

  default:
    break;
  }

  return obj;
}
