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
double solver_CosAngleBetweenVectors(const double *v1, const double *v2)
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
int SOLVER_GetNeighborByVector(const LB_Lattice_p lattice, int node, const double *vector)
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
 * Scalar multiplication of two 3D vectors
 */
double solver_scalarVectorMultiply(double *v1, double *v2)
{
  return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

/*
 * Calculate feq by Bhatnager, Gross, Krook model
 */
void solver_feqBHK(LB_Lattice_p lattice, double *fnew, double density, double *velocity)
{
  int i;
  double *current_vector = solver_GetVectors(lattice->node_type);

  for (i = 0; i < lattice->node_type; ++i, current_vector += 3)
  {
    double A = 1;
    double B = 3;
    double C = 4.5;
    double D = -1.5;
    double t;

    t = solver_scalarVectorMultiply(current_vector, velocity);
    fnew[i] = A + B * t;
    t *= t;
    fnew[i] += C * t;
    t = solver_scalarVectorMultiply(velocity, velocity);
    t *= t;
    fnew[i] += D * t;
    fnew[i] /= density;
  }
}

/*
 * Calculate f equilibrium
 */
void solver_feq(LB_Lattice_p lattice, double *fnew, double density, double *velocity)
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
  
  for (i = 0; i < nodes_cnt; ++i)
  {
    double density = 0;
    double u[3];
    double tau = 3 * lattice->nodes[i].viscosity + 0.5;
    int k = 0;
    int forces_num = objects[0].recalculate_force(&(objects[0]), NULL, 0, forces);
    double *current_vector = solver_GetVectors(lattice->node_type);
    double fe[3] = {0, 0, 0};
    double *fnew = new double[lattice->node_type];
    
    uint xpos, ypos, zpos;
    double x, y, z;
    

    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    
    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;
    
    for (k = 0; k < lattice->node_type; ++k, current_vector += 3)
    {
      density += lattice->fs[i*3 + k];
      fe[0] += lattice->fs[i*3 + k] * current_vector[0];
      fe[1] += lattice->fs[i*3 + k] * current_vector[1];
      fe[2] += lattice->fs[i*3 + k] * current_vector[2];
    }
    u[0] = fe[0] / density;
    u[1] = fe[1] / density;
    u[2] = fe[2] / density;
    
    lattice->vectors[i * lattice->node_type] = u[0] + u[1] + u[2];
    
    solver_feq(lattice, fnew, density, u);

    for (k = 0; k < lattice->node_type; ++k, current_vector += 3)
    {
      double delta = (lattice->fs[i*3 + k] - fnew[k]) / tau;
      //printf("Node %.3d; k %.3d; delta:%f (%f - %f)\n", i, k, delta, lattice->fs[i*3 + k], fnew[k]);
      lattice->fs[i*3 + k] -= delta;
    }
    
    for (int j = 0; j < forces_num; ++j)
    {
      double B = 3;
      current_vector = solver_GetVectors(lattice->node_type);
      
      double dist = 0;
      dist += (forces[j].points[0] - x) * (forces[j].points[0] - x);
      dist += (forces[j].points[1] - y) * (forces[j].points[1] - y);
      dist += (forces[j].points[2] - z) * (forces[j].points[2] - z);
      dist = sqrt(dist) + 0.1;
    
      for (k = 0; k < lattice->node_type; ++k, current_vector += 3)
      {
        double ztau = ((2*tau - 1) / 2*tau) * B;
        double zvm = solver_scalarVectorMultiply(current_vector, forces[j].vector);
        double delta = ztau * zvm / (100 * dist);
        
        if (i == 30 && k == 0)
        {
          printf("Node %.3d; k %.3d; force delta:%f\n", i, k, delta);
        }
        lattice->fs[i*3 + k] += delta;
      }
    }
    delete[] fnew;
  }

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

/*
 * Calculate lattice parameters with time delta = dt
 */
void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt)
{
  //solver_ResolveNonPhysical(lattice, objects, objnum, dt);
  solver_ResolveLBGeneric(lattice, objects, objnum, dt);
}
