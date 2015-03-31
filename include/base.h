#ifndef BASE_H
#define BASE_H

#include <extobj.h>

#define LB_IS_LATTICE_HEX(lattice_type) ((lattice_type) == LB_LATTICE_2D_HEX || (lattice_type) == LB_LATTICE_3D_HEX)

typedef enum {
	LB_CALC_CPU = 0,
	LB_CALC_OPENCL_CPU,
	LB_CALC_OPENCL_GPU,
	LB_CALC_MAX
} LB_CalcType_t;

typedef struct {
	lb_float max_velocity;
} BASE_statistics_t, *BASE_statistics_p;

void BASE_Stop();
void BASE_MoveObjects(EXTOBJ_obj_set_p obj_set, lb_float dx, lb_float dy,
		      lb_float dz);
void BASE_ForcesSwitch(EXTOBJ_obj_set_p obj_set, int on);
void BASE_GetPosByIdx(const LB_Lattice_p lattice, int index, int * x, int * y, int * z);
int BASE_GetIdxByPos(const LB_Lattice_p lattice, int x, int y, int z);

void BASE_SetCalcType(LB_CalcType_t type);
LB_CalcType_t BASE_GetCalcType(void);

void BASE_SetCurrentObjectSet(EXTOBJ_obj_set_p obj_set);
EXTOBJ_obj_set_p BASE_GetCurrentObjectSet(void);

lb_float BASE_GetTimeMs(long time_start, long time_stop);
LB_Lattice_t *LB_CreateLattice(LB_lattice_type_t lattice_type,
			       LB_node_type_t node_type, int countX,
			       int countY, int countZ, lb_float sizeX,
			       lb_float sizeY, lb_float sizeZ);

void BASE_GetStatistics(const LB_Lattice_p lattice, BASE_statistics_p stat);

#endif				/* BASE_H */
