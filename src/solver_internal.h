#ifndef SOLVER_INTERNAL_H
#define SOLVER_INTERNAL_H

int solver_initOpencl(void);

typedef struct solver_vector_s {
	lb_float x;
	lb_float y;
	lb_float z;
	lb_float omega;
} solver_vector_t, *solver_vector_p;

typedef struct {
	int forces_num;
	EXTOBJ_force_t forces[100];
} force_pack_t, *force_pack_p;

solver_vector_p solver_GetVectors(LB_node_type_t type);

int solver_ResolveOpencl(LB_Lattice_p lattice, force_pack_p forces,
			 int forces_num);

#endif				/* SOLVER_INTERNAL_H */
