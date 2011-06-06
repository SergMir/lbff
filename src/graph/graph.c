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
#include <stdlib.h>

/* ------------------------------- Defines --------------------------------- */

#define RERP_VECTORS

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

void graph_GetColorByVelocity(lb_float velocity, lb_float * r, lb_float * g, lb_float * b);

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

/*
 * Window resize callback
 */
void graph_Reshape(int width, int height)
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
  glutReshapeFunc(graph_Reshape);
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
 * Simple points or vectors visualisation
 */
void graph_GetColorByVelocity(lb_float velocity, lb_float * r, lb_float * g, lb_float * b)
{
  lb_float max_color_1 = 217.0f, max_color_2 = 232.0f, max_color_3 = 255.0f,
           min_color_1 = 0,      min_color_2 = 2.0f,   min_color_3 = 6.0f,
           clm = 255.0f;

  if ((velocity >= 1.0))
  {
    *r = 225.0f / clm;
    *g = 237.0f / clm;
    *b = 255.0f / clm;

    return;
  }
  else if (velocity < 0.00001)
  {
    *r = 0;
    *g = 0;
    *b = 0;
  }
  else
  {
    if (((velocity * max_color_1) / clm)< (min_color_1 / clm))
    {
      *r = 0; //min_color_1/clm;
    }
    else
    {
      *r = (velocity * max_color_1) / clm;
    }
    
    if ((velocity * max_color_2) / clm < (min_color_2 / clm))
    {
      *g = 0; //min_color_2/clm;
    }
    else
    {
      *g = (velocity * max_color_2) / clm;
    }
    
    if ((velocity * max_color_3) / clm < min_color_3 / clm)
    {
      *b = 0; //min_color_3/clm;
    }
    else
    {
      *b = (velocity * max_color_3) / clm;
    }
  }
  return;
}

/*
 * Draw objects
 */
void graph_RedrawObjects(const EXTOBJ_obj_set_p obj_set)
{
  uint i;
  
  glColor3f(0, 1.0f, 0);

  glBegin(GL_LINES);
  for (i = 0; i < obj_set->count; ++i)
  {
    EXTOBJ_obj_p objects = obj_set->objects;
    LB3D_p pos = &(objects[i].pos);
    int j = 0;
    for (j = 0; j < objects[i].points_cnt - 1; ++j)
    {
      glVertex2f(pos->x + objects[i].points[j].x,
                 pos->y + objects[i].points[j].y);
      
      glVertex2f(pos->x + objects[i].points[j+1].x,
                 pos->y + objects[i].points[j+1].y);
    }
    
    glVertex2f(pos->x + objects[i].points[j].x,
               pos->y + objects[i].points[j].y);
    glVertex2f(pos->x + objects[i].points[0].x,
               pos->y + objects[i].points[0].y);
  }
  glEnd();
}

/*
 * Simple points or vectors visualisation
 */
void GRAPH_RedrawSimple(const LB_Lattice_p lattice, const EXTOBJ_obj_set_p obj_set)
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

  graph_RedrawObjects(obj_set);

  glFlush();
  glutSwapBuffers(); 
}

/*
 * Solid fill visualization.
 */
void GRAPH_RedrawSolid(const LB_Lattice_p lattice, const EXTOBJ_obj_set_p obj_set)
{
  LB3D_p ch_vector = lattice->velocities;
  lb_float minv = 1000000, maxv = 0, avgv = 0;
  static lb_float new_max = 0;

  glClear(GL_COLOR_BUFFER_BIT);

  glColor3f(0.0f, 0.0f, 1.0f);
  glLineWidth(1);
  glPointSize(4);

  glBegin(GL_POINTS);

  lb_float ** array = (lb_float **) malloc(sizeof (lb_float*) * lattice->countY);
  for (uint i = 0; i < lattice->countY; i++)
    array[i] = (lb_float *) malloc(sizeof (lb_float) * lattice->countX);

  for (uint ypos = 0; ypos < lattice->countY; ypos++)
  {
    for (uint xpos = 0; xpos < lattice->countX; xpos++)
    {
      lb_float rel_velocity, v_sum = 0;

      v_sum = sqrt
              (ch_vector->x * ch_vector->x +
               ch_vector->y * ch_vector->y +
               ch_vector->z * ch_vector->z);
      ch_vector++;
      minv = v_sum < minv ? v_sum : minv;
      maxv = v_sum > maxv ? v_sum : maxv;
      avgv = v_sum;

      rel_velocity = min(1.0, v_sum / new_max);
      rel_velocity = min(1.0, v_sum / 2.0);
      lb_float d = rel_velocity;

      lb_float dx = lattice->velocities[xpos * ypos].x > 0 ? d : -d;
      dx *= lattice->sizeX / lattice->countX;
      lb_float dy = lattice->velocities[xpos * ypos].y > 0 ? d : -d;
      dy *= lattice->sizeY / lattice->countY;

      array[ypos][xpos] = sqrt(dx * dx + dy * dy);
    }
  }

  for (uint ypos = 0; ypos < lattice->countY - 1; ypos++)
  {
    for (uint xpos = 0; xpos < lattice->countX - 1; xpos++)
    {
      lb_float x, y;
      x = xpos * lattice->sizeX / lattice->countX;
      y = ypos * lattice->sizeY / lattice->countY;
      lb_float red, green, blue;
      lb_float sq1 = array[ypos][xpos];
      graph_GetColorByVelocity(sq1, &red, &green, &blue);
      glColor3f(red, green, blue);
      glVertex2f(x, y);

      lb_float sq2 = array[ypos][xpos + 1];
      lb_float sq3 = array[ypos + 1][xpos];
      lb_float sq4 = array[ypos + 1][xpos + 1];

      lb_float sq_2 = (2.0 * sq1 + sq2) / 3.0;
      graph_GetColorByVelocity(sq_2, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 1.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ypos * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_3 = (sq1 + sq2 * 2.0) / 3.0;
      graph_GetColorByVelocity(sq_3, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 2.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ypos * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_4 = (2.0 * sq1 + sq3) / 3.0;
      graph_GetColorByVelocity(sq_4 / new_max, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = (xpos) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 1.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_5 = (sq1 + 2.0 * sq3) / 3.0;
      graph_GetColorByVelocity(sq_5, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = (xpos) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 2.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_6 = (sq_4 + sq_2) / 2.0;
      graph_GetColorByVelocity(sq_6, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 1.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 1.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_7 = (sq_6 + sq_3) / 2.0;
      graph_GetColorByVelocity(sq_7, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 2.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 1.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_8 = (sq_5 + sq_6) / 2.0;
      graph_GetColorByVelocity(sq_8, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 1.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 2.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);

      lb_float sq_9 = (sq_8 + sq4) / 2.0;
      graph_GetColorByVelocity(sq_9, &red, &green, &blue);
      glColor3f(red, green, blue);
      x = ((lb_float) xpos + 2.0 / 3.0) * lattice->sizeX / lattice->countX;
      y = ((lb_float) ypos + 2.0 / 3.0) * lattice->sizeY / lattice->countY;
      glVertex2f(x, y);
    }
  }
  glEnd();

  printf("min %10.4f, max %10.4f, avg %10.4f\n", minv, maxv, avgv);
  new_max = maxv;

  graph_RedrawObjects(obj_set);

  glFlush();
  glutSwapBuffers();

  for (uint i = 0; i < lattice->countY; i++)
    realloc((void *) array[i], sizeof (lb_float) * lattice->countX);
  realloc((void *) array, sizeof (lb_float *) * lattice->countY);
}

/*
 * Main draw function: draw given lattice and objects
 */
void GRAPH_Redraw(const LB_Lattice_p lattice, const EXTOBJ_obj_set_p obj_set)
{
  //GRAPH_RedrawSimple(lattice, obj_set);
  GRAPH_RedrawSolid(lattice, obj_set);
}
