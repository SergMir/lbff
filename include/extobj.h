#ifndef EXTOBJ_H
#define EXTOBJ_H

typedef struct
{
  LB3D_t points;
  LB3D_t vector;
  lb_float  force;
} EXTOBJ_force_t;

typedef enum
{
  EXTOBJ_TYPE_SIMPLE,
  EXTOBJ_TYPE_LAST
} EXTOBJ_obj_type_t;

/* Take incoming forces and its number, calculate generated forces, return its number */
typedef int (*EXTOBJ_force_calculate)(void *obj, const EXTOBJ_force_t *input_forces, uint in_forces_num,
        EXTOBJ_force_t *output_forces);

typedef struct
{
  LB3D_p points;
  int points_cnt;
  int turnedOn;
  EXTOBJ_force_calculate recalculate_force;
} EXTOBJ_obj_t, *EXTOBJ_obj_p;

typedef struct
{
  uint capacity;
  uint count;
  EXTOBJ_obj_p objects;
} EXTOBJ_obj_set_t, *EXTOBJ_obj_set_p;

EXTOBJ_obj_set_p EXTOBJ_CreateObjectSet(int capacity);
void EXTOBJ_AddObject(EXTOBJ_obj_set_p obj_set, EXTOBJ_obj_type_t type);

#endif /* EXTOBJ_H */

