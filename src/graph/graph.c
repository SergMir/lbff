/*
 * LBFF: Graphical output module
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
#include <graph.h>

#include <GL/glut.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 * Window resize callback
 */
void Reshape(int width, int height)
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-10, 100, -10, 100);
  glMatrixMode(GL_MODELVIEW);
}

/*
 * Register callbacks, create OpenGL window
 */
int GRAPH_Init(f_mainloop_t mainloop)
{
  glutReshapeFunc(Reshape);
  glutDisplayFunc(mainloop);
  glClearColor(0, 0, 0, 0);

  glViewport(0, 0, 400, 300);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-10, 100, -10, 100);
  glMatrixMode(GL_MODELVIEW);

  glutMainLoop();
  return 0;
}

/*
 * Main draw function: draw given lattice and objects
 */
void GRAPH_Redraw(LB_Lattice_p lattice, EXTOBJ_obj_p objects, uint objcnt)
{
  uint i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  double *ch_vector = lattice->vectors;

  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0f, 0.0f, 1.0f);
  glLineWidth(1);
  glPointSize(5);

  glBegin(GL_POINTS);

  for (i = 0; i < nodes_cnt; ++i)
  {
    double x, y, z, j;
    double rel_velocity, v_sum = 0;
    uint xpos, ypos, zpos;
    float red, blue;
    
    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    

    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;

    for (j = 0; j < lattice->node_type; ++j)
    {
      v_sum += *(ch_vector++);
    }
    rel_velocity = min(1.0, v_sum / 1.0);
    red = rel_velocity;
    blue = 1.0 - red;
    glColor3f(red, 0.0f, blue);
    glVertex2f(x, y);
  }
  glEnd();

  glColor3f(0, 1.0f, 0);

  glBegin(GL_LINES);
  for (i = 0; i < objcnt; ++i)
  {
    int j = 0;
    for (j = 0; j < objects[i].points_cnt - 1; ++j)
    {
      glVertex2f(objects[i].points[ j    * 3 + 0],
                 objects[i].points[ j    * 3 + 1]);
      
      glVertex2f(objects[i].points[(j+1) * 3 + 0],
                 objects[i].points[(j+1) * 3 + 1]);
    }
    
    glVertex2f(objects[i].points[j * 3 + 0],
               objects[i].points[j * 3 + 1]);
    glVertex2f(objects[i].points[0],
               objects[i].points[1]);
  }
  glEnd();

  glFlush();
  glutSwapBuffers(); 
}
