#ifndef EXTOBJ_H
#define EXTOBJ_H

typedef struct
{
  double points[3];
  double vector[3];
  double  force;
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
  double *points;
  int points_cnt;
  int turnedOn;
  EXTOBJ_force_calculate recalculate_force;
} EXTOBJ_obj_t, *EXTOBJ_obj_p;

EXTOBJ_obj_p EXTOBJ_CreateObject(EXTOBJ_obj_type_t type);

#endif /* EXTOBJ_H */

