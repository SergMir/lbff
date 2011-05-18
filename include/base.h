#ifndef BASE_H
#define BASE_H

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))
#define LB_IS_LATTICE_HEX(lattice_type) ((lattice_type) == LB_LATTICE_2D_HEX || (lattice_type) == LB_LATTICE_3D_HEX)

void BASE_Stop();
void BASE_MoveObjects(lb_float dx, lb_float dy, lb_float dz);
void BASE_ForcesSwitch(int on);
void BASE_GetPosByIdx(const LB_Lattice_p lattice, int index, uint *x, uint *y, uint *z);
long BASE_GetTimeNs(void);
lb_float BASE_GetTimeMs(long time_start, long time_stop);
LB_Lattice_t* LB_CreateLattice(LB_lattice_type_t lattice_type, LB_node_type_t node_type,
                               uint countX, uint countY, uint countZ,
                               lb_float sizeX, lb_float sizeY, lb_float sizeZ);

#endif /* BASE_H */
