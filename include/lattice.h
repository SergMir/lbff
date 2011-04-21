#ifndef BASE_LATTICE_H
#define BASE_LATTICE_H

#include <sys/types.h>

typedef enum
{
  LB_LATTICE_2D_SQUARE,
  LB_LATTICE_2D_HEX,
  LB_LATTICE_2D_LAST,

  LB_LATTICE_3D_CUBE,
  LB_LATTICE_3D_HEX,
  LB_LATTICE_LAST
} LB_lattice_type_t;

typedef enum
{
  LB_NODE_D2_Q5 =   4,
  LB_NODE_D2_Q9 =   8,
  LB_NODE_D3_Q7 =   6,
  LB_NODE_D3_Q15 = 14,
  LB_NODE_D3_Q19 = 18,
  LB_NODE_LAST
} LB_node_type_t;

typedef struct LB_Node_s
{
  double density;
  double viscosity;
} LB_Node_t;

typedef struct
{
  LB_Node_t *nodes;
  double *vectors;  /* Velocities for each characteristic vector for each node */
  double *fs;       /* Particle distribution for -/- */
  LB_lattice_type_t lattice_type;
  LB_node_type_t node_type;

  uint countX;
  uint countY;
  uint countZ;

  double sizeX;
  double sizeY;
  double sizeZ;

} LB_Lattice_t, *LB_Lattice_p;

#endif /* BASE_LATTICE_H */
