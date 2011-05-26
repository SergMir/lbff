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
#include <base.h>
#include <solver.h>

#include <stdlib.h>
#include <time.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

extern int flag_stop;
static EXTOBJ_obj_set_p current_obj_set = NULL;

static LB_CalcType_t calc_type = LB_CALC_CPU; 

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
  uint i = 0;
  for (i = 0; i < BASE_GetCurrentObjectSet()->count; ++i)
  {
    BASE_GetCurrentObjectSet()->objects[i].turnedOn = on;
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
    EXTOBJ_obj_p objects = BASE_GetCurrentObjectSet()->objects;
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

/*
 * Real-time clock in nanoseconds
 */
long BASE_GetTimeNs(void)
{
  struct timespec tp;
  clock_gettime(CLOCK_REALTIME, &tp);
  return tp.tv_sec * 1000000000 + tp.tv_nsec;
}

/*
 * Get difference in milliseconds
 */
lb_float BASE_GetTimeMs(long time_start, long time_stop)
{
  return (time_stop - time_start) / 1000000.0;
}

/*
 * Set current device type for calculations
 */
void BASE_SetCalcType(LB_CalcType_t type)
{
  if ((LB_CALC_MAX > type) && (0 <= type))
  {
    calc_type = type;
  }
}

/*
 * Get current device type for calculations
 */
LB_CalcType_t BASE_GetCalcType(void)
{
  return calc_type;
}

/*
 * Set current working set of external objects
 */
void BASE_SetCurrentObjectSet(EXTOBJ_obj_set_p obj_set)
{
  current_obj_set = obj_set;
}

/*
 * Get current working set of external objects
 */
EXTOBJ_obj_set_p BASE_GetCurrentObjectSet(void)
{
  return current_obj_set;
}
