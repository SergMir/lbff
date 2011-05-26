#ifndef SOLVER_H
#define SOLVER_H

int SOLVER_Init(void);
void SOLVER_InitLattice(LB_Lattice_p lattice);

void SOLVER_Resolve(LB_Lattice_p lattice, EXTOBJ_obj_set_p obj_set, LB_CalcType_t calc_type, lb_float dt);

#endif /* SOLVER_H */

