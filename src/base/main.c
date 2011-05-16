/*
 * LBFF: Main module
 *
 * Copyright (C) 2011 LBFF Authors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation: http://www.gnu.org/licenses/
 */
#include <lattice.h>
#include <base.h>
#include <extobj.h>
#include <solver.h>
#include <graph.h>
#include <ui.h>

#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <unistd.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

LB_Lattice_t *lattice = NULL;
EXTOBJ_obj_p objects = NULL;

int objects_cnt = 0;
int flag_stop = 0;

/* --------------------------- Implementation ------------------------------ */

/*
 * Fluid recalculation and redraw
 */
void mainLoop()
{
  lb_float time_start, time_stop, dt = 0.1;

  time_start = time(NULL);

  SOLVER_Resolve(lattice, objects, 1, dt);
  GRAPH_Redraw(lattice, objects, 1);

  time_stop = time(NULL);
  dt = time_stop - time_start;

  if (flag_stop)
  {
    exit(0);
  }
  else
  {
    glutPostRedisplay();
  }
}

/*
 *
 */
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(100, 100);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutCreateWindow("LBFF");
  glutFullScreen();

  UI_Init();
  SOLVER_Init();
  lattice = LB_CreateLattice(LB_LATTICE_2D_SQUARE, LB_NODE_D2_Q9,
                             90, 90, 1,
                             90, 90, 1);
  objects = EXTOBJ_CreateObject(EXTOBJ_TYPE_SIMPLE);
  ++objects_cnt;
  GRAPH_Init(mainLoop);

  return 0;
}

