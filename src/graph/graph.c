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
#include <stdio.h>
#include <math.h>

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
void GRAPH_Redraw(const LB_Lattice_p lattice, const EXTOBJ_obj_p objects, uint objcnt)
{
  uint i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  LB3D_p ch_vector = lattice->velocities;
  double minv = 1000000, maxv = 0, avgv = 0;
  static double new_max = 0;

  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0f, 0.0f, 1.0f);
  glLineWidth(1);
  glPointSize(4);

  //glBegin(GL_POINTS);
  glBegin(GL_LINES);

  for (i = 0; i < nodes_cnt; ++i)
  {
    double x, y, z;
    double rel_velocity, v_sum = 0;
    uint xpos, ypos, zpos;
    float red, blue;

    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    

    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;

    v_sum = fabs(ch_vector->x) + fabs(ch_vector->y) + fabs(ch_vector->z);
    ch_vector++;
    minv = v_sum < minv ? v_sum : minv;
    maxv = v_sum > maxv ? v_sum : maxv;
    avgv += v_sum;
    rel_velocity = min(1.0, v_sum / new_max);
    rel_velocity = min(1.0, v_sum / 3.0);
    red = rel_velocity;
    blue = 1.0 - red;
    glColor3f(red, 0.0f, blue);
    glVertex2f(x, y);
    double d = rel_velocity;
    double dx = lattice->velocities[i].x > 0 ? d : -d;
    double dy = lattice->velocities[i].y > 0 ? d : -d;
    glVertex2f(x + dx, y + dy);
  }
  glEnd();
  
  avgv /= nodes_cnt;
  printf("min %10.4f, max %10.4f, avg %10.4f\n", minv, maxv, avgv);
  new_max = maxv;

  glColor3f(0, 1.0f, 0);

  glBegin(GL_LINES);
  for (i = 0; i < objcnt; ++i)
  {
    int j = 0;
    for (j = 0; j < objects[i].points_cnt - 1; ++j)
    {
      glVertex2f(objects[i].points[j].x,
                 objects[i].points[j].y);
      
      glVertex2f(objects[i].points[j+1].x,
                 objects[i].points[j+1].y);
    }
    
    glVertex2f(objects[i].points[j].x,
               objects[i].points[j].y);
    glVertex2f(objects[i].points[0].x,
               objects[i].points[0].y);
  }
  glEnd();

  glFlush();
  glutSwapBuffers(); 
}
