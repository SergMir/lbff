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
#include <utils.h>

#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>
#include <unistd.h>

#define LB_FULLSCREEN
#undef LB_FULLSCREEN

LB_Lattice_t *lattice = NULL;
EXTOBJ_obj_set_p obj_set = NULL;

int objects_cnt = 0;
int flag_stop = 0;

struct ui_ctx *ui_ctx = NULL;
struct ui_label *stat_label = NULL;

static void keypress_callback(int key)
{
	static int forces_on = 0;
	lb_float dx = 0, dy = 0;

	switch (key) {
	case KEY_ESC:
		BASE_Stop();
		break;

	case KEY_SPACE:
		forces_on = !forces_on;
		BASE_ForcesSwitch(BASE_GetCurrentObjectSet(), forces_on);
		break;

	case KEY_ENTER:
		if (LB_CALC_OPENCL_CPU == BASE_GetCalcType())
			BASE_SetCalcType(LB_CALC_CPU);
		else
			BASE_SetCalcType(LB_CALC_OPENCL_CPU);
		break;

	case GLUT_KEY_LEFT:
		dx = -1;
		dy = 0;
		break;

	case GLUT_KEY_RIGHT:
		dx = 1;
		dy = 0;
		break;

	case GLUT_KEY_UP:
		dx = 0;
		dy = 1;
		break;

	case GLUT_KEY_DOWN:
		dx = 0;
		dy = -1;
		break;

	default:
		break;
	}

	switch (key) {
	case GLUT_KEY_LEFT:
	case GLUT_KEY_RIGHT:
	case GLUT_KEY_UP:
	case GLUT_KEY_DOWN:
		BASE_MoveObjects(BASE_GetCurrentObjectSet(), dx, dy, 0);
		break;
	}
}

void mainLoop()
{
	int dt = 0.1, dt_resolved, dt_rendered;
	long time_start, time_resolved, time_rendered;
	BASE_statistics_t stat;
	char dbg_string[256];

	time_start = util_get_time();

	SOLVER_Resolve(lattice, obj_set, BASE_GetCalcType(), dt);

	time_resolved = util_get_time();

	GRAPH_RenderWorld(lattice, obj_set);
	ui_render(ui_ctx);

	GRAPH_FinishRender();

	time_rendered = util_get_time();

	dt_resolved = util_diff_time_us(time_start, time_resolved);
	dt_rendered = util_diff_time_us(time_resolved, time_rendered);
	dt = dt_resolved + dt_rendered;
	//printf("Calculation: %8.3f ms; Rendering: %8.3f ms; Summary: %8.3f ms\n", dt_resolved, dt_rendered, dt);
	BASE_GetStatistics(lattice, &stat);
	sprintf(dbg_string, "Max velocity: %8.3f", stat.max_velocity);
	ui_set_label_text(stat_label, dbg_string);

	if (flag_stop) {
		exit(0);
	} else {
		glutPostRedisplay();
	}
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("LBFF");
#if defined(LB_FULLSCREEN)
	glutFullScreen();
#endif

	ui_ctx = ui_init(keypress_callback);
	stat_label = ui_create_label(ui_ctx, 1, 90, 60, 5, "Label sample");
	SOLVER_Init();
	lattice = LB_CreateLattice(LB_LATTICE_2D_SQUARE, LB_NODE_D2_Q9,
				   90, 90, 1, 90, 90, 1);
	obj_set = EXTOBJ_CreateObjectSet(100);
	BASE_SetCurrentObjectSet(obj_set);
	LB3D_t pos = { 10, 10, 0 };
	EXTOBJ_AddObject(obj_set, &pos, EXTOBJ_TYPE_SIMPLE);
	++objects_cnt;
	GRAPH_Init(mainLoop);

	return 0;
}
