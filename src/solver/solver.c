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
  return 0;
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

#define BHK_VAR 1

/*
 * Calculate feq by Bhatnager, Gross, Krook model
 */
void solver_feqBHK(LB_Lattice_p lattice, double *fnew, double density, LB3D_p velocity)
{
  int i;
  solver_vector_p vector = solver_GetVectors(lattice->node_type);

  for (i = 0; i < lattice->node_type; ++i)
  {
#if BHK_VAR == 0
    double c = 1;
    double teta = c * c / 3;
    double A = 1;
    double B = 1 / teta;
    double C = 1 / (2 * teta * teta);
    double D = - 1 / (2 * teta);
    double t;

    t = solver_scalarVectorMultiply((LB3D_p)(vector + i), velocity);
    fnew[i] = A + B * t + C * t * t + D * solver_scalarVectorMultiply(velocity, velocity);
    fnew[i] *= vector[i].omega;
    fnew[i] *= density;
#elif BHK_VAR == 1
    double c = 1;
    double teta = c * c / 3;
    double A = density;
    double B = 1 / teta;
    double C = 1 / (2 * teta * teta);
    double D = - 1 / (2 * teta);
    double t;

    t = solver_scalarVectorMultiply((LB3D_p)(vector + i), velocity);
    fnew[i] = A + B * t + C * t * t + D * solver_scalarVectorMultiply(velocity, velocity);
    fnew[i] *= vector[i].omega;
#elif BHK_VAR == 2
    double c = 1;
    double teta = c * c / 3;
    double A = density;
    double B = 1 / teta;
    double C = 1 / (2 * teta * teta);
    double D = - 1 / (2 * teta);
    double t;

    t = solver_scalarVectorMultiply((LB3D_p)(vector + i), velocity);
    fnew[i] = A + B * t + D * t * t + C * solver_scalarVectorMultiply(velocity, velocity);
    fnew[i] *= vector[i].omega;
#endif
  }
}

/*
 * Calculate f equilibrium
 */
void solver_feq(LB_Lattice_p lattice, double *fnew, double density, LB3D_p velocity)
{
  return solver_feqBHK(lattice, fnew, density, velocity);
}

/*
 * Calculate with generic LBM
 */
void solver_ResolveLBGeneric(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  int i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  static EXTOBJ_force_t forces[1000];
  double *feqs = new double[lattice->node_type];
  int forces_num = objects[0].recalculate_force(&(objects[0]), NULL, 0, forces);
  solver_vector_p current_vector = solver_GetVectors(lattice->node_type);
  double tau = 1.5;
  
  memset(lattice->fs + nodes_cnt * lattice->node_type,
         0,
         sizeof(double) * nodes_cnt * lattice->node_type);

  for (i = 0; i < nodes_cnt; ++i)
  {
    LB3D_p u = lattice->velocities + i;
    double density = 0;
    LB3D_t fe = {0, 0, 0};
    
    int k = 0;
    uint xpos, ypos, zpos;
    double x, y, z;

    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    
    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;

    for (k = 0; k < lattice->node_type; ++k)
    {
      double fs = lattice->fs[i * lattice->node_type + k];
      density += fs;
      fe.x += fs * current_vector[k].x;
      fe.y += fs * current_vector[k].y;
      fe.z += fs * current_vector[k].z;
    }
    u->x = fe.x / density;
    u->y = fe.y / density;
    u->z = fe.z / density;

    solver_feq(lattice, feqs, density, u);

    for (k = 0; k < lattice->node_type; ++k)
    {
      int next_node = SOLVER_GetNeighborByVector(lattice, i, current_vector + k);

      if (next_node != -1)
      {
        double fs = lattice->fs[i*lattice->node_type + k];
        double delta = (fs - feqs[k]) / tau;
        lattice->fs[nodes_cnt * lattice->node_type + next_node * lattice->node_type + k] += fs - delta;
      }
    }
  }
  
  for (i = 0; i < nodes_cnt; ++i)
  {
    int k;
    for (k = 0; k < lattice->node_type; ++k)
    {
      int next_node = SOLVER_GetNeighborByVector(lattice, i, current_vector + k);

      if (next_node == -1)
      {
        int opp_k;
        solver_vector_p opp_current_vector = solver_GetVectors(lattice->node_type);
        for (opp_k = 0; opp_k < lattice->node_type; ++opp_k)
        {
          if (
            F_COMP(current_vector[k].x, -opp_current_vector[opp_k].x) &&
            F_COMP(current_vector[k].y, -opp_current_vector[opp_k].y) &&
            F_COMP(current_vector[k].z, -opp_current_vector[opp_k].z))
          {
            break;
          }
        }
        if (opp_k < lattice->node_type)
        {
          lattice->fs[nodes_cnt * lattice->node_type + i * lattice->node_type + opp_k] += lattice->fs[nodes_cnt * lattice->node_type + i * lattice->node_type + k];
          lattice->fs[nodes_cnt * lattice->node_type + i * lattice->node_type + k] = 0;   
        }
      }
    }
  }
  
  
  for (int j = 0; j < forces_num; ++j)
  {
    double B = 3, mindist = 10e5;
    int k, mini = 0;

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
      double ztau = ((2*tau - 1) / (2*tau)) * B;
      double zvm = solver_scalarVectorMultiply((LB3D_p)(current_vector + k), &(forces[j].vector));
      double delta = ztau * zvm * 100000.1;
      if (!F_COMP(delta, 0))
      {
        lattice->fs[nodes_cnt * lattice->node_type + mini*lattice->node_type + k] += delta;
      }
    }
  }
  
  delete[] feqs;
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
