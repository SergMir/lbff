/*
 * LBFF: Solver - fluids intercating, collision and moving calculations
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
#include <solver.h>

#include "solver_internal.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 *
 */
int SOLVER_Init()
{
  return 0;
}

/*
 * Get set of characteristic directions vectors from node type
 */
double* solver_GetVectors(LB_node_type_t type)
{
  double *vectors = NULL;

  switch(type)
  {
  case LB_NODE_D2_Q5:
    vectors = (double*)solver_vectors_D2_Q5;
    break;
  case LB_NODE_D2_Q9:
    vectors = (double*)solver_vectors_D2_Q9;
    break;
  case LB_NODE_D3_Q7:
    vectors = (double*)solver_vectors_D3_Q7;
    break;
  case LB_NODE_D3_Q15:
    vectors = (double*)solver_vectors_D3_Q15;
    break;
  case LB_NODE_D3_Q19:
    vectors = (double*)solver_vectors_D3_Q19;
    break;
  default:
    break;
  }

  return vectors;
}

/*
 * Cosinus of angle between vectors
 */
double solver_CosAngleBetweenVectors(double *v1, double *v2)
{
  double lv1 = sqrt(v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2]);
  double lv2 = sqrt(v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2]);
  double l = sqrt(lv1 * lv2);
  double ab = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];

  return ab / l;
}

/*
 * Get nearest node vector pointing at from given node
 */
int SOLVER_GetNeighborByVector(LB_Lattice_p lattice, int node, double *vector)
{
  int dx = fabs(vector[0]) > 0.577 ? 1 : 0;
  int dy = fabs(vector[1]) > 0.577 ? 1 : 0;
  int dz = fabs(vector[2]) > 0.577 ? 1 : 0;
  uint xpos, ypos, zpos;

  BASE_GetPosByIdx(lattice, node, &xpos, &ypos, &zpos);

  dx *= vector[0] > 0 ? 1 : -1;
  dy *= vector[1] > 0 ? 1 : -1;
  dz *= vector[2] > 0 ? 1 : -1;

  xpos += xpos < lattice->countX - 1 ? dx : 0;
  ypos += ypos < lattice->countY - 1 ? dy : 0;
  zpos += zpos < lattice->countZ - 1 ? dz : 0;

  node = zpos * lattice->countX * lattice->countY + ypos * lattice->countX + xpos;

  return node;
}

/*
 * Calculate lattice parameters with time delta = dt
 */
void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  int i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  static EXTOBJ_force_t forces[1000];

  for (i = 0; i < nodes_cnt; ++i)
  {
    uint xpos, ypos, zpos;
    int k = 0, forces_num = objects[0].recalculate_force(&(objects[0]), NULL, 0, forces);
    double *current_vector = solver_GetVectors(lattice->node_type);
    double x, y, z;

    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    
    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;


    for (; k < lattice->node_type; ++k, current_vector += 3)
    {
      int j = 0;
      double delta;
      
      for (; j < forces_num; ++j)
      {
        double cosfi = solver_CosAngleBetweenVectors(current_vector, forces[j].vector);
        double dist = 0;

        dist += (forces[j].points[0] - x) * (forces[j].points[0] - x);
        dist += (forces[j].points[1] - y) * (forces[j].points[1] - y);
        dist += (forces[j].points[2] - z) * (forces[j].points[2] - z);
        dist = sqrt(dist) + 0.1;

        if (cosfi > 0)
        {
          double delta_force = 0.05 * forces[j].force * cosfi / (lattice->nodes[i].density * dist);
          lattice->vectors[i * lattice->node_type + k] += max(delta_force, 0);
        }


        lattice->vectors[i * lattice->node_type + k] *= 9.95 * dt;
      }

      delta = 0.01 * lattice->vectors[i * lattice->node_type + k];

      lattice->vectors[i * lattice->node_type + k] -= delta;
      lattice->vectors[SOLVER_GetNeighborByVector(lattice, i, current_vector) * lattice->node_type + k] += delta;
    }
  }
  
  objnum = objnum;
}
