#ifndef BASE_H
#define BASE_H

#define max(a, b) ((a > b) ? (a) : (b))
#define min(a, b) ((a < b) ? (a) : (b))
#define LB_IS_LATTICE_HEX(lattice_type) ((lattice_type) == LB_LATTICE_2D_HEX || (lattice_type) == LB_LATTICE_3D_HEX)

void BASE_Stop();
void BASE_MoveObjects(double dx, double dy, double dz);
void BASE_ForcesSwitch(int on);
void BASE_GetPosByIdx(LB_Lattice_p lattice, int index, int *x, int *y, int *z);
LB_Lattice_t* LB_CreateLattice(LB_lattice_type_t lattice_type, LB_node_type_t node_type,
                               uint countX, uint countY, uint countZ,
                               double sizeX, double sizeY, double sizeZ);

#endif /* BASE_H */
