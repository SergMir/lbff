/*
 * LBFF: Utilites for world manipulating
 *
 * Copyright (C) 2011 LBFF Authors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation: http://www.gnu.org/licenses/
 */
#include <lattice.h>
#include <extobj.h>
#include <solver.h>

#include <stdlib.h>
#include <time.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

extern EXTOBJ_obj_p objects;
extern int flag_stop;
extern int objects_cnt;

/* --------------------------- Implementation ------------------------------ */

/*
 * Stop fluid recalculations and exit from main loop
 */
void BASE_Stop()
{
  flag_stop = 1;
}

/*
 * Turn on/off active force-generation objects (engines, etc.)
 */
void BASE_ForcesSwitch(int on)
{
  int i = 0;
  for (i = 0; i < objects_cnt; ++i)
  {
    objects[i].turnedOn = on;
  }
}

/*
 * Get xyz indexes of node in lattice by it's index
 */
void BASE_GetPosByIdx(const LB_Lattice_p lattice, int index, uint *x, uint *y, uint *z)
{
  uint xy = lattice->countX * lattice->countY;

  *z = index / xy;
  index -= *z * xy;
  *y = index / lattice->countX;
  *x = index - *y * lattice->countX;
}

/*
 * Move objects on xyz deltas
 */
void BASE_MoveObjects(lb_float dx, lb_float dy, lb_float dz)
{
  int i = 0, j = 0;

  for (i = 0; i < 1; ++i)
  {
    for (j = 0; j < objects[i].points_cnt; ++j)
    {
      objects[i].points[j].x += dx;
      objects[i].points[j].y += dy;
      objects[i].points[j].z += dz;
    }
  }
}

/*
 * Lattice constructor
 */
LB_Lattice_t* LB_CreateLattice(LB_lattice_type_t lattice_type,
                               LB_node_type_t node_type,
                               uint countX, uint countY, uint countZ,
                               lb_float sizeX, lb_float sizeY, lb_float sizeZ)
{
  LB_Lattice_t *lattice = (LB_Lattice_t *)malloc(sizeof(LB_Lattice_t));
  int nodes_num = countX * countY * countZ;
  
  lattice->lattice_type = lattice_type;
  lattice->node_type = node_type;

  lattice->countX = countX;
  lattice->countY = countY;
  lattice->countZ = countZ;

  lattice->sizeX = sizeX;
  lattice->sizeY = sizeY;
  lattice->sizeZ = sizeZ;

  lattice->nodes   =      (LB_Node_t*)malloc(sizeof(LB_Node_t) * nodes_num);
  lattice->velocities =   (LB3D_p)malloc(nodes_num * sizeof(LB3D_t));
  lattice->fs =           (lb_float *)malloc(2 * sizeof(lb_float) * nodes_num * node_type);
  lattice->openCLparams =  NULL;

  SOLVER_InitLattice(lattice);

  return lattice;
}

long BASE_GetTimeNs(void)
{
  struct timespec tp;
  clock_gettime (CLOCK_REALTIME, &tp);
  return tp.tv_sec * 1000000000 + tp.tv_nsec;
}

lb_float BASE_GetTimeMs(long time_start, long time_stop)
{
  return (time_stop - time_start) / 1000000.0;
}
