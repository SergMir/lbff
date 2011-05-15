#ifndef SOLVER_H
#define SOLVER_H

int SOLVER_Init(void);
void SOLVER_InitLattice(LB_Lattice_p lattice);

void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_p objects, int objnum, double dt);

#endif /* SOLVER_H */

