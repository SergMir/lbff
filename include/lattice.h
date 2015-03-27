#ifndef BASE_LATTICE_H
#define BASE_LATTICE_H

#include <sys/types.h>

typedef float lb_float;

typedef struct LB3D_s {
	lb_float x;
	lb_float y;
	lb_float z;
} LB3D_t, *LB3D_p;

typedef enum {
	LB_LATTICE_2D_SQUARE,
	LB_LATTICE_2D_HEX,
	LB_LATTICE_2D_LAST,

	LB_LATTICE_3D_CUBE,
	LB_LATTICE_3D_HEX,
	LB_LATTICE_LAST
} LB_lattice_type_t;

typedef enum {
	LB_NODE_D2_Q5 = 5,
	LB_NODE_D2_Q9 = 9,
	LB_NODE_D3_Q7 = 7,
	LB_NODE_D3_Q15 = 15,
	LB_NODE_D3_Q19 = 19,
	LB_NODE_LAST
} LB_node_type_t;

typedef struct LB_Node_s {
	lb_float density;
	lb_float viscosity;
} LB_Node_t;

typedef struct LB_OpenCL_s {
	void *u;
	void *fs;
	void *fsnew;
	void *vectors;
	void *forces;
	void *params;
} LB_OpenCL_t, *LB_OpenCL_p;

typedef struct {
	LB_Node_t *nodes;
	LB3D_p velocities;	/* Velocities for each characteristic vector for each node */
	lb_float *fs;		/* Particle distribution for -/- */
	LB_lattice_type_t lattice_type;
	LB_node_type_t node_type;
	LB_OpenCL_p openCLparams;

	uint countX;
	uint countY;
	uint countZ;

	lb_float sizeX;
	lb_float sizeY;
	lb_float sizeZ;

	struct {
		lb_float max_velocity;
		lb_float min_velocity;
		lb_float avg_velocity;
	} statistics;

} LB_Lattice_t, *LB_Lattice_p;

#endif				/* BASE_LATTICE_H */
