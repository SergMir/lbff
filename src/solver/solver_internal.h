#ifndef SOLVER_INTERNAL_H
#define SOLVER_INTERNAL_H

int solver_initOpencl(void);

typedef struct solver_vector_s
{
  double x;
  double y;
  double z;
  double omega;
} solver_vector_t, *solver_vector_p;

solver_vector_p solver_GetVectors(LB_node_type_t type);
        
#endif /* SOLVER_INTERNAL_H */

