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
#include <string.h>

/* ------------------------------- Defines --------------------------------- */

#define F_COMP(X, Y) (fabs((X) - (Y)) < 0.0000000001)

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 *
 */
int SOLVER_Init()
{
  int status = 0;
  
  //status = solver_initOpencl();
  
  return status;
}

/*
 * Get set of characteristic directions vectors from node type
 */
solver_vector_p solver_GetVectors(LB_node_type_t type)
{
  solver_vector_p vectors = NULL;

  switch(type)
  {
  case LB_NODE_D2_Q5:
    vectors = solver_vectors_D2_Q5;
    break;
  case LB_NODE_D2_Q9:
    vectors = solver_vectors_D2_Q9;
    break;
  case LB_NODE_D3_Q7:
    vectors = solver_vectors_D3_Q7;
    break;
  case LB_NODE_D3_Q15:
    vectors = solver_vectors_D3_Q15;
    break;
  case LB_NODE_D3_Q19:
    vectors = solver_vectors_D3_Q19;
    break;
  default:
    break;
  }

  return vectors;
}

/*
 * Cosinus of angle between vectors
 */
double solver_CosAngleBetweenVectors(const LB3D_p v1, const LB3D_p v2)
{
  double lv1 = sqrt(v1->x*v1->x + v1->y*v1->y + v1->z*v1->z);
  double lv2 = sqrt(v2->x*v2->x + v2->y*v2->y + v2->z*v2->z);
  double l = sqrt(lv1 * lv2);
  double ab = v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;

  return ab / l;
}

/*
 * Get nearest node vector pointing at from given node
 */
int SOLVER_GetNeighborByVector(const LB_Lattice_p lattice, int node, const solver_vector_p vector)
{
  int dx = fabs(vector->x) > 0.577 ? 1 : 0;
  int dy = fabs(vector->y) > 0.577 ? 1 : 0;
  int dz = fabs(vector->z) > 0.577 ? 1 : 0;
  uint xpos, ypos, zpos;

  BASE_GetPosByIdx(lattice, node, &xpos, &ypos, &zpos);

  dx *= vector->x > 0 ? 1 : -1;
  dy *= vector->y > 0 ? 1 : -1;
  dz *= vector->z > 0 ? 1 : -1;
  
  do
  {
    node = -1;
    
    if (dx < 0 && xpos == 0)
      break;
    
    if (dx > 0 && xpos == (lattice->countX - 1))
      break;
    
    if (dy < 0 && ypos == 0)
      break;
    
    if (dy > 0 && ypos == (lattice->countY - 1))
      break;
    
    if (dz < 0 && zpos == 0)
      break;
    
    if (dz > 0 && zpos == (lattice->countZ - 1))
      break;
    
    xpos += dx;
    ypos += dy;
    zpos += dz;
    
    node = zpos * lattice->countX * lattice->countY + ypos * lattice->countX + xpos;
  } while (0);
  
  return node;
}

/*
 * Scalar multiplication of two 3D vectors
 */
double solver_scalarVectorMultiply(LB3D_p v1, LB3D_p v2)
{
  return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

/*
 * Fill lattice with liqud in state of equilibrium
 */
void SOLVER_InitLattice(LB_Lattice_p lattice)
{
  int i = 0, nodes_num = lattice->countX * lattice->countY * lattice->countZ;
  solver_vector_p vector = solver_GetVectors(lattice->node_type);
  
  for (i = 0; i < nodes_num; ++i)
  {
    int j;
    double *fs_vector = lattice->fs + i * lattice->node_type;
    LB3D_p u = lattice->velocities + i;
    
    u->x = 0;
    u->y = 0;
    u->z = 0;

    for (j = 0; j < lattice->node_type; ++j)
    {
      fs_vector[j] = vector[j].omega;
    }
  }
}

#define BHK_VAR 0

/*
 * Calculate feq by Bhatnager, Gross, Krook model
 */
double solver_feqBHK(LB_Lattice_p lattice, double density, LB3D_p velocity, solver_vector_p vector)
{
  double fnew;

#if BHK_VAR == 0
    double c = 1;
    double teta = c * c / 3;
    double A = 1;
    double B = 1 / teta;
    double C = 1 / (2 * teta * teta);
    double D = - 1 / (2 * teta);
    double t;

    t = solver_scalarVectorMultiply((LB3D_p)vector, velocity);
    fnew = A + B * t + C * t * t + D * solver_scalarVectorMultiply(velocity, velocity);
    fnew *= vector->omega;
    fnew *= density;
#endif

    lattice = lattice;
    return fnew;
}

/*
 * Calculate f equilibrium
 */
double solver_feq(LB_Lattice_p lattice, double density, LB3D_p velocity, solver_vector_p vector)
{
  return solver_feqBHK(lattice, density, velocity, vector);
}

/*
 * Calculate with generic LBM
 */
void solver_ResolveLBGeneric(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  int i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  
  
  solver_vector_p vector = solver_GetVectors(lattice->node_type);
  double tau = 0.7;
  double *fsn = lattice->fs + nodes_cnt * lattice->node_type;
  
  memset(fsn,
         0,
         sizeof(double) * nodes_cnt * lattice->node_type);

  for (i = 0; i < nodes_cnt; ++i)
  {
    LB3D_p u = lattice->velocities + i;
    double density = 0;
    LB3D_t fe = {0, 0, 0};
    double *fsi = lattice->fs + i * lattice->node_type;
    
    int k = 0;

    for (k = 0; k < lattice->node_type; ++k)
    {
      double fs = fsi[k];
      density += fs;
      fe.x += fs * vector[k].x;
      fe.y += fs * vector[k].y;
      fe.z += fs * vector[k].z;
    }
    u->x = fe.x / density;
    u->y = fe.y / density;
    u->z = fe.z / density;

    for (k = 0; k < lattice->node_type; ++k)
    {
      int next_node = SOLVER_GetNeighborByVector(lattice, i, vector + k);

      if (next_node != -1)
      {
        double feq = solver_feq(lattice, density, u, vector + k);
        double delta = (fsi[k] - feq) / tau;
        fsn[next_node * lattice->node_type + k] += fsi[k] - delta;
      }
    }
  }
  
  for (i = 0; i < nodes_cnt; ++i)
  {
    int k;
    for (k = 0; k < lattice->node_type; ++k)
    {
      int next_node = SOLVER_GetNeighborByVector(lattice, i, vector + k);

      if (next_node == -1)
      {
        int opp_k;
        solver_vector_p opp_vector = solver_GetVectors(lattice->node_type);
        for (opp_k = 0; opp_k < lattice->node_type; ++opp_k)
        {
          if (
            F_COMP(vector[k].x, -opp_vector[opp_k].x) &&
            F_COMP(vector[k].y, -opp_vector[opp_k].y) &&
            F_COMP(vector[k].z, -opp_vector[opp_k].z))
          {
            break;
          }
        }
        if (opp_k < lattice->node_type)
        {
          fsn[i * lattice->node_type + opp_k] += lattice->fs[i * lattice->node_type + k];
        }
      }
    }
  }
  
  for (int obj = 0; obj < objnum; ++obj)
  {
    static EXTOBJ_force_t forces[1000];
    int forces_num = objects[obj].recalculate_force(&(objects[obj]), NULL, 0, forces);
    
    for (int j = 0; j < forces_num; ++j)
    {
      double B = 3, mindist = 10e5;
      int k, mini = 0;
      double density = 0;

      for (i = 0; i < nodes_cnt; ++i)
      {
        double dist = 0;
        uint xpos, ypos, zpos;
        double x, y, z;

        BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);

        x = xpos * lattice->sizeX / lattice->countX;
        y = ypos * lattice->sizeY / lattice->countY;
        z = zpos * lattice->sizeZ / lattice->countZ;
        dist += (forces[j].points.x - x) * (forces[j].points.x - x);
        dist += (forces[j].points.y - y) * (forces[j].points.y - y);
        dist += (forces[j].points.z - z) * (forces[j].points.z - z);
        dist = sqrt(dist);

        if (dist < mindist)
        {
          mindist = dist;
          mini = i;
        }
      }

      for (k = 0; k < lattice->node_type; ++k)
      {
        double fs = fsn[mini * lattice->node_type + k];
        density += fs;
      }

      for (k = 0; k < lattice->node_type; ++k)
      {
#if 0
        double ztau = ((2 * tau - 1) / (2 * tau)) * B;
        double zvm = solver_scalarVectorMultiply((LB3D_p) (vector + k), &(forces[j].vector));
        double delta = ztau * zvm * 100000.1;
        if (!F_COMP(delta, 0))
        {
          fsn[mini * lattice->node_type + k] += delta;
        }
#else
        LB3D_t nvec = {
          vector[k].x - lattice->velocities[mini].x,
          vector[k].y - lattice->velocities[mini].y,
          vector[k].z - lattice->velocities[mini].z
        };
        double delta = solver_scalarVectorMultiply(&(forces[j].vector), &nvec);
        delta *= solver_feq(lattice, density, &(lattice->velocities[mini]), vector + k);
        fsn[mini * lattice->node_type + k] += delta;
        B= B;
#endif
      }
    }
  }
  
  memcpy(lattice->fs, lattice->fs + nodes_cnt * lattice->node_type, sizeof(double) * nodes_cnt * lattice->node_type);

  lattice = lattice;
  objects = objects;
  objnum = objnum;
  dt = dt;
}

/*
 * Calculate with own simple non-realistic "physics"
 */
void solver_ResolveNonPhysical(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  int i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  static EXTOBJ_force_t forces[1000];

  for (i = 0; i < nodes_cnt; ++i)
  {
    uint xpos, ypos, zpos;
    int k = 0, forces_num = objects[0].recalculate_force(&(objects[0]), NULL, 0, forces);
    solver_vector_p current_vector = solver_GetVectors(lattice->node_type);
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
        double cosfi = solver_CosAngleBetweenVectors((LB3D_p)current_vector, &(forces[j].vector));
        double dist = 0;

        dist += (forces[j].points.x - x) * (forces[j].points.x - x);
        dist += (forces[j].points.y - y) * (forces[j].points.y - y);
        dist += (forces[j].points.z - z) * (forces[j].points.z - z);
        dist = sqrt(dist) + 0.1;

        if (cosfi > 0)
        {
          double delta_force = 0.05 * forces[j].force * cosfi / (lattice->nodes[i].density * dist);
          lattice->velocities[i * lattice->node_type + k].x += max(delta_force, 0);
        }


        lattice->velocities[i * lattice->node_type + k].x *= 9.95 * dt;
      }

      delta = 0.01 * lattice->velocities[i * lattice->node_type + k].x;

      lattice->velocities[i * lattice->node_type + k].x -= delta;
      lattice->velocities[SOLVER_GetNeighborByVector(lattice, i, current_vector) * lattice->node_type + k].x += delta;
    }
  }
  
  objnum = objnum;
}

/*
 * Calculate lattice parameters with time delta = dt
 */
void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  //solver_ResolveNonPhysical(lattice, objects, objnum, dt);
  solver_ResolveLBGeneric(lattice, objects, objnum, dt);
}
