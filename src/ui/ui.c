/*
 * LBFF: User Interface, both graphical and console
 *
 * Copyright (C) 2011 LBFF Authors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation: http://www.gnu.org/licenses/
 */
#include <lattice.h>
#include <base.h>
#include <ui.h>
#include <extobj.h>

#include <GL/glut.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 * Alphanumerical keys handler
 */
void UI_KeyboardHandler(unsigned char key, int x, int y)
{
  static int forces_on = 0;

  switch (key)
  {
  case 27:
    BASE_Stop();
    break;

  case 32:
    forces_on = !forces_on;
    BASE_ForcesSwitch(forces_on);
    break;
    
  default:
    break;
  }
  
  /* Anti-Warning */
  x = x;
  y = y;
}

/*
 * Special keys handler
 */
void UI_SpecKeyboardHandler(int key, int x, int y)
{
  double dx, dy;

  switch (key)
  {
  case GLUT_KEY_LEFT:
    dx = -1;
    dy =  0;
    break;
  case GLUT_KEY_RIGHT:
    dx =  1;
    dy =  0;
    break;
  case GLUT_KEY_UP:
    dx =  0;
    dy =  1;
    break;
  case GLUT_KEY_DOWN:
    dx =  0;
    dy = -1;
    break;
  }

  switch (key)
  {
  case GLUT_KEY_LEFT:
  case GLUT_KEY_RIGHT:
  case GLUT_KEY_UP:
  case GLUT_KEY_DOWN:
    BASE_MoveObjects(dx, dy, 0);
    break;
  }
  
  /* Anti-Warning */
  x = x;
  y = y;
}

/*
 * Register keys callbacks
 */
int UI_Init()
{
  glutKeyboardFunc(UI_KeyboardHandler);
  glutSpecialFunc(UI_SpecKeyboardHandler);
  return 0;
}

