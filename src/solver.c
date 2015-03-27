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

#define F_COMP(X, Y) (fabs((X) - (Y)) < 0.00001)

/* c = 1.0 / sqrt(3.0) */
#define c 0.57735

solver_vector_t solver_vectors_D2_Q5[] = {
	{0.0, 0.0, 0.0, 2.0 / 6.0},

	{1.0, 0.0, 0.0, 1.0 / 6.0},
	{0.0, 1.0, 0.0, 1.0 / 6.0},
	{-1.0, 0.0, 0.0, 1.0 / 6.0},
	{0.0, -1.0, 0.0, 1.0 / 6.0}
};

solver_vector_t solver_vectors_D2_Q9[] = {
	{0.0, 0.0, 0.0, 4.0 / 9.0},

	{1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, 1.0, 0.0, 1.0 / 9.0},
	{-1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, -1.0, 0.0, 1.0 / 9.0},

	{1.0, 1.0, 0.0, 1.0 / 36.0},
	{-1.0, 1.0, 0.0, 1.0 / 36.0},
	{-1.0, -1.0, 0.0, 1.0 / 36.0},
	{1.0, -1.0, 0.0, 1.0 / 36.0}
};

solver_vector_t solver_vectors_D3_Q7[] = {
	{0.0, 0.0, 0.0, 3.0 / 9.0},

	{1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, 1.0, 0.0, 1.0 / 9.0},
	{-1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, -1.0, 0.0, 1.0 / 9.0},
	{0.0, 0.0, 1.0, 1.0 / 9.0},
	{0.0, 0.0, -1.0, 1.0 / 9.0},
};

solver_vector_t solver_vectors_D3_Q15[] = {
	{0.0, 0.0, 0.0, 2.0 / 9.0},

	{1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, 1.0, 0.0, 1.0 / 9.0},
	{-1.0, 0.0, 0.0, 1.0 / 9.0},
	{0.0, -1.0, 0.0, 1.0 / 9.0},
	{0.0, 0.0, 1.0, 1.0 / 9.0},
	{0.0, 0.0, -1.0, 1.0 / 9.0},

	{1.0, 1.0, 1.0, 1.0 / 72.0},
	{1.0, 1.0, -1.0, 1.0 / 72.0},
	{-1.0, 1.0, 1.0, 1.0 / 72.0},
	{-1.0, 1.0, -1.0, 1.0 / 72.0},
	{-1.0, -1.0, 1.0, 1.0 / 72.0},
	{-1.0, -1.0, -1.0, 1.0 / 72.0},
	{1.0, -1.0, 1.0, 1.0 / 72.0},
	{1.0, -1.0, -1.0, 1.0 / 72.0},
};

solver_vector_t solver_vectors_D3_Q19[] = {
	{0.0, 0.0, 0.0, 1.0 / 3.0},

	{1.0, 0.0, 0.0, 1.0 / 18.0},
	{0.0, 1.0, 0.0, 1.0 / 18.0},
	{-1.0, 0.0, 0.0, 1.0 / 18.0},
	{0.0, -1.0, 0.0, 1.0 / 18.0},
	{0.0, 0.0, 1.0, 1.0 / 18.0},
	{0.0, 0.0, -1.0, 1.0 / 18.0},

	{1.0, 1.0, 0.0, 1.0 / 36.0},
	{-1.0, 1.0, 0.0, 1.0 / 36.0},
	{-1.0, -1.0, 0.0, 1.0 / 36.0},
	{1.0, -1.0, 0.0, 1.0 / 36.0},
	{0.0, 1.0, 1.0, 1.0 / 36.0},
	{0.0, -1.0, 1.0, 1.0 / 36.0},
	{0.0, -1.0, -1.0, 1.0 / 36.0},
	{0.0, 1.0, -1.0, 1.0 / 36.0},
	{1.0, 0.0, 1.0, 1.0 / 36.0},
	{1.0, 0.0, -1.0, 1.0 / 36.0},
	{-1.0, 0.0, -1.0, 1.0 / 36.0},
	{-1.0, 0.0, 1.0, 1.0 / 36.0},
};

/*
 *
 */
int SOLVER_Init()
{
	int status = 0;

	status = solver_initOpencl();
	if (status != 0) {
		exit(-1);
	}

	return status;
}

/*
 * Get set of characteristic directions vectors from node type
 */
solver_vector_p solver_GetVectors(LB_node_type_t type)
{
	solver_vector_p vectors = NULL;

	switch (type) {
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
lb_float solver_CosAngleBetweenVectors(const LB3D_p v1, const LB3D_p v2)
{
	lb_float lv1 = sqrt(v1->x * v1->x + v1->y * v1->y + v1->z * v1->z);
	lb_float lv2 = sqrt(v2->x * v2->x + v2->y * v2->y + v2->z * v2->z);
	lb_float l = sqrt(lv1 * lv2);
	lb_float ab = v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;

	return ab / l;
}

/*
 * Get nearest node vector pointing at from given node
 */
int SOLVER_GetNeighborByVector(const LB_Lattice_p lattice, int node,
			       const solver_vector_p vector)
{
	int dx = fabs(vector->x) > 0.577 ? 1 : 0;
	int dy = fabs(vector->y) > 0.577 ? 1 : 0;
	int dz = fabs(vector->z) > 0.577 ? 1 : 0;
	uint xpos, ypos, zpos;

	BASE_GetPosByIdx(lattice, node, &xpos, &ypos, &zpos);

	dx *= vector->x > 0 ? 1 : -1;
	dy *= vector->y > 0 ? 1 : -1;
	dz *= vector->z > 0 ? 1 : -1;

	do {
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

		node =
		    zpos * lattice->countX * lattice->countY +
		    ypos * lattice->countX + xpos;
	} while (0);

	return node;
}

/*
 * Scalar multiplication of two 3D vectors
 */
lb_float solver_scalarVectorMultiply(LB3D_p v1, LB3D_p v2)
{
	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

/*
 * Fill lattice with liqud in state of equilibrium
 */
void SOLVER_InitLattice(LB_Lattice_p lattice)
{
	int i = 0, nodes_num =
	    lattice->countX * lattice->countY * lattice->countZ;
	solver_vector_p vector = solver_GetVectors(lattice->node_type);

	for (i = 0; i < nodes_num; ++i) {
		uint j;
		lb_float *fs_vector = lattice->fs + i * lattice->node_type;
		LB3D_p u = lattice->velocities + i;

		u->x = 0;
		u->y = 0;
		u->z = 0;

		for (j = 0; j < lattice->node_type; ++j) {
			fs_vector[j] = vector[j].omega;
		}
	}
}

#define BHK_VAR 0

/*
 * Calculate feq by Bhatnager, Gross, Krook model
 */
lb_float solver_feqBHK(LB_Lattice_p lattice, lb_float density, LB3D_p velocity,
		       solver_vector_p vector)
{
	lb_float fnew;

#if BHK_VAR == 0
	lb_float teta = c * c;
	lb_float A = 1;
	lb_float B = 1 / teta;
	lb_float C = 1 / (2 * teta * teta);
	lb_float D = -1 / (2 * teta);
	lb_float t;

	t = solver_scalarVectorMultiply((LB3D_p) vector, velocity);
	fnew =
	    A + B * t + C * t * t + D * solver_scalarVectorMultiply(velocity,
								    velocity);
	fnew *= vector->omega;
	fnew *= density;
#endif

	lattice = lattice;
	return fnew;
}

/*
 * Calculate f equilibrium
 */
lb_float solver_feq(LB_Lattice_p lattice, lb_float density, LB3D_p velocity,
		    solver_vector_p vector)
{
	return solver_feqBHK(lattice, density, velocity, vector);
}

/*
 * Calculate with generic LBM
 */
void solver_ResolveLBGeneric(LB_Lattice_p lattice, EXTOBJ_obj_set_p obj_set,
			     LB_CalcType_t calc_type, lb_float dt)
{
	int nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
	static force_pack_t forces[100];

	lb_float tau = 0.55;
	lb_float *fsn = lattice->fs + nodes_cnt * lattice->node_type;

	memset(fsn, 0, sizeof(lb_float) * nodes_cnt * lattice->node_type);
	tau = tau;

	for (uint obj = 0; obj < obj_set->count; ++obj) {
		forces[obj].forces_num =
		    obj_set->objects[obj].
		    recalculate_force(&(obj_set->objects[obj]), NULL, 0,
				      forces[obj].forces);
	}

	if (LB_CALC_OPENCL_CPU == calc_type) {
		solver_ResolveOpencl(lattice, forces, obj_set->count);
	} else if (LB_CALC_CPU == calc_type) {
		int i;
		solver_vector_p vector = solver_GetVectors(lattice->node_type);

		for (i = 0; i < nodes_cnt; ++i) {
			LB3D_p u = lattice->velocities + i;
			lb_float density = 0;
			LB3D_t fe = { 0, 0, 0 };
			lb_float *fsi = lattice->fs + i * lattice->node_type;
			uint k = 0;
			uint xpos, ypos, zpos;
			lb_float x, y, z;

			BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);

			x = xpos * lattice->sizeX / lattice->countX;
			y = ypos * lattice->sizeY / lattice->countY;
			z = zpos * lattice->sizeZ / lattice->countZ;

			for (k = 0; k < lattice->node_type; ++k) {
				lb_float fs = fsi[k];
				density += fs;
				fe.x += fs * vector[k].x;
				fe.y += fs * vector[k].y;
				fe.z += fs * vector[k].z;
			}
			u->x = fe.x / density;
			u->y = fe.y / density;
			u->z = fe.z / density;

			if (c < sqrt(u->x * u->x + u->y * u->y + u->z * u->z)) {
				for (k = 0; k < lattice->node_type; ++k) {
					u->x = 0;
					u->y = 0;
					u->z = 0;
					density = 1.0;
					fsi[k] =
					    solver_feq(lattice, density, u,
						       vector + k);
				}
			}

			for (k = 0; k < lattice->node_type; ++k) {
				int next_node =
				    SOLVER_GetNeighborByVector(lattice, i,
							       vector + k);

				if (next_node != -1) {
					lb_float feq =
					    solver_feq(lattice, density, u,
						       vector + k);
					lb_float delta = (fsi[k] - feq) / tau;
					fsn[next_node * lattice->node_type +
					    k] += fsi[k] - delta;
				} else {
					uint opp_k;
					solver_vector_p opp_vector =
					    solver_GetVectors(lattice->
							      node_type);
					for (opp_k = 0;
					     opp_k < lattice->node_type;
					     ++opp_k) {
						if (F_COMP
						    (vector[k].x,
						     -opp_vector[opp_k].x)
						    && F_COMP(vector[k].y,
							      -opp_vector
							      [opp_k].y)
						    && F_COMP(vector[k].z,
							      -opp_vector
							      [opp_k].z)) {
							break;
						}
					}
					if (opp_k < lattice->node_type) {
						fsn[i * lattice->node_type +
						    opp_k] += fsi[k];
					}
				}
			}

			for (uint obj = 0; obj < obj_set->count; ++obj) {
				for (int j = 0; j < forces[obj].forces_num; ++j) {
					lb_float dx =
					    forces[obj].forces[j].points.x - x;
					lb_float dy =
					    forces[obj].forces[j].points.y - y;
					lb_float dz =
					    forces[obj].forces[j].points.z - z;
					lb_float dist =
					    sqrt(dx * dx + dy * dy + dz * dz);

					lb_float d =
					    3 * lattice->sizeX /
					    lattice->countX;

					if (dist < d) {
						for (k = 0;
						     k < lattice->node_type;
						     ++k) {
							lb_float delta =
							    ((d -
							      dist) / d) *
							    solver_scalarVectorMultiply
							    ((LB3D_p)
							     (vector + k),
							     &(forces[obj].
							       forces[j].
							       vector));
							if (fsn
							    [i *
							     lattice->
							     node_type + k] +
							    delta < 0) {
								fsn[i *
								    lattice->
								    node_type +
								    k] = 0;
							} else {
								fsn[i *
								    lattice->
								    node_type +
								    k] += delta;
							}
						}
					}
				}
			}
		}
	}

	memcpy(lattice->fs, lattice->fs + nodes_cnt * lattice->node_type,
	       sizeof(lb_float) * nodes_cnt * lattice->node_type);

	dt = dt;
}

/*
 * Calculate lattice parameters with time delta = dt
 */
void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_set_p objects,
		    LB_CalcType_t calc_type, lb_float dt)
{
	solver_ResolveLBGeneric(lattice, objects, calc_type, dt);
}
