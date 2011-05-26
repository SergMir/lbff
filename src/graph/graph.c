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

#define RERP_VECTORS

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
void GRAPH_Redraw(const LB_Lattice_p lattice, const EXTOBJ_obj_set_p obj_set)
{
  uint i, nodes_cnt = lattice->countX * lattice->countY * lattice->countZ;
  LB3D_p ch_vector = lattice->velocities;
  lb_float minv = 1000000, maxv = 0, avgv = 0;
  static lb_float new_max = 0;

  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0f, 0.0f, 1.0f);
  glLineWidth(1);
  glPointSize(4);

#if defined(RERP_VECTORS)
  glBegin(GL_LINES);
#else
  glBegin(GL_POINTS);
#endif

  for (i = 0; i < nodes_cnt; ++i)
  {
    lb_float x, y, z;
    lb_float rel_velocity, v_sum = 0;
    uint xpos, ypos, zpos;
    lb_float red, blue;

    BASE_GetPosByIdx(lattice, i, &xpos, &ypos, &zpos);
    

    x = xpos * lattice->sizeX / lattice->countX;
    y = ypos * lattice->sizeY / lattice->countY;
    z = zpos * lattice->sizeZ / lattice->countZ;

    v_sum = sqrt(
            ch_vector->x * ch_vector->x +
            ch_vector->y * ch_vector->y +
            ch_vector->z * ch_vector->z);
    ch_vector++;
    minv = v_sum < minv ? v_sum : minv;
    maxv = v_sum > maxv ? v_sum : maxv;
    avgv += v_sum;
    rel_velocity = min(1.0, v_sum / new_max);
    //rel_velocity = min(1.0, v_sum / 2.0);
    red = rel_velocity;
    blue = 1.0 - red;
    glColor3f(red, 0.0f, blue);
    glVertex2f(x, y);
  #if defined(RERP_VECTORS)
    lb_float dx = lattice->velocities[i].x / new_max;
    dx *= lattice->sizeX / lattice->countX;
    lb_float dy = lattice->velocities[i].y / new_max;
    dy *= lattice->sizeY / lattice->countY;
    glVertex2f(x + dx, y + dy);
  #endif
  }
  glEnd();
  
  avgv /= nodes_cnt;
  printf("min %10.4f, max %10.4f, avg %10.4f\n", minv, maxv, avgv);
  new_max = maxv;

  glColor3f(0, 1.0f, 0);

  glBegin(GL_LINES);
  for (i = 0; i < obj_set->count; ++i)
  {
    EXTOBJ_obj_p objects = obj_set->objects;
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
