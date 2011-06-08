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
#include <graph.h>

#include <GL/glut.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------------- Defines --------------------------------- */

/* -------------------------------- Types ---------------------------------- */

/* --------------------------- Local Routines ------------------------------ */

/* ------------------------------- Globals --------------------------------- */

/* --------------------------- Implementation ------------------------------ */

typedef struct ui_buttons_s
{
  UI_button_t button;
  struct ui_buttons_s *next;
} ui_buttons_t, *ui_buttons_p;

ui_buttons_p ui_buttons_head = NULL;

typedef struct ui_labels_s
{
  UI_label_t label;
  struct ui_labels_s *next;
} ui_labels_t, *ui_labels_p;

ui_labels_p ui_labels_head = NULL;

/*
 * Alphanumerical keys handler
 */
void UI_KeyboardHandler(unsigned char key, int x, int y)
{
  static int forces_on = 0;

  switch (key)
  {
  case 27: //ESC
    BASE_Stop();
    break;

  case 32: //SPACE
    forces_on = !forces_on;
    BASE_ForcesSwitch(BASE_GetCurrentObjectSet(), forces_on);
    break;
    
  case 13: //ENTER
  {
    /*
    LB_CalcType_t type = BASE_GetCalcType() + 1;
    if (LB_CALC_MAX == type)
    {
      type = 0;
    }*/
    if (LB_CALC_OPENCL_CPU == BASE_GetCalcType())
    {
      BASE_SetCalcType(LB_CALC_CPU);
    }
    else
    {
      BASE_SetCalcType(LB_CALC_OPENCL_CPU);
    }
  }
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
  lb_float dx, dy;

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
    BASE_MoveObjects(BASE_GetCurrentObjectSet(), dx, dy, 0);
    break;
  }
  
  /* Anti-Warning */
  x = x;
  y = y;
}

void ui_MouseHandler(int button, int state, int sx, int sy)
{
  ui_buttons_p cbutton;
  GLdouble x, y, z;
  
  GLint   viewport[4];
  GLdouble projection[16];
  GLdouble modelview[16];
  
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  
  gluUnProject(sx, sy, 0, modelview, projection, viewport, &x, &y, &z);
  
  y = 100.0 - y;

  for (cbutton = ui_buttons_head; NULL != cbutton; cbutton = cbutton->next)
  {
    UI_button_p button = &cbutton->button;

    if (x >= button->x &&
      x <= button->x + button->width &&
      y >= button->y &&
      y <= button->y + button->height)
    {
      break;
    }
  }
  
  if (NULL != cbutton && GLUT_LEFT_BUTTON == button)
  {
    cbutton->button.callback(GLUT_DOWN == state ? UI_ACT_PRESSED : UI_ACT_RELEASED);
  }
}

/*
 * 
 */
UI_button_p UI_CreateButton(lb_float x, lb_float y, lb_float width, lb_float height, const char *text, UI_buttonAction callback)
{
  ui_buttons_p button, button_prev = NULL;
  
  for (button = ui_buttons_head; NULL != button; button_prev = button, button = button->next);
  
  button = (ui_buttons_p)malloc(sizeof(ui_buttons_t));
  if (NULL == button_prev)
  {
    ui_buttons_head = button;
  }
  else
  {
    button_prev->next = button;
  }
  
  button->button.x = x;
  button->button.y = y;
  button->button.width = width;
  button->button.height = height;
  button->button.callback = callback;
  button->next = NULL;
  strcpy(button->button.text, text);
  
  return &button->button;
}

/*
 * 
 */
UI_label_p UI_CreateLabel(lb_float x, lb_float y, lb_float width, lb_float height, const char *text)
{
  ui_labels_p label, label_prev = NULL;
  
  for (label = ui_labels_head; NULL != label; label_prev = label, label = label->next);
  
  label = (ui_labels_p)malloc(sizeof(ui_labels_t));
  if (NULL == label_prev)
  {
    ui_labels_head = label;
  }
  else
  {
    label_prev->next = label;
  }
  
  label->label.x = x;
  label->label.y = y;
  label->label.width = width;
  label->label.height = height;
  label->next = NULL;
  strcpy(label->label.text, text);
  
  return &label->label;
}

/*
 * 
 */
void UI_ChangeTextLabel(UI_label_p label)
{
  label = label;
}

/*
 * 
 */
void UI_Draw(void)
{
  ui_buttons_p button;
  ui_labels_p label;

  for (button = ui_buttons_head; NULL != button; button = button->next)
  {
    GRAPH_DrawButton(
                     button->button.x,
                     button->button.y,
                     button->button.width,
                     button->button.height,
                     button->button.text);
  }

  for (label = ui_labels_head; NULL != label; label = label->next)
  {
    GRAPH_DrawLabel(
                    label->label.x,
                    label->label.y,
                    label->label.width,
                    label->label.height,
                    label->label.text);
  }
}

/*
 * Test button callback handler
 */
void ui_testButoonHandler(UI_action_t action)
{
  printf("Test button, action: %d\n", action);
}

/*
 * Register keys callbacks
 */
int UI_Init()
{
  UI_CreateLabel(1, 90, 20, 5, "Label sample");
  UI_CreateButton(30, 90, 20, 5, "Button sample", ui_testButoonHandler);
  glutKeyboardFunc(UI_KeyboardHandler);
  glutSpecialFunc(UI_SpecKeyboardHandler);
  glutMouseFunc(ui_MouseHandler);
  return 0;
}

