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

UI_label_p stat_label = NULL;

/* --------------------------- Implementation ------------------------------ */

/*
 * Fluid recalculation and redraw
 */
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
	UI_Draw();

	GRAPH_FinishRender();

	time_rendered = util_get_time();

	dt_resolved = util_diff_time_us(time_start, time_resolved);
	dt_rendered = util_diff_time_us(time_resolved, time_rendered);
	dt = dt_resolved + dt_rendered;
	//printf("Calculation: %8.3f ms; Rendering: %8.3f ms; Summary: %8.3f ms\n", dt_resolved, dt_rendered, dt);
	BASE_GetStatistics(lattice, &stat);
	sprintf(dbg_string, "Max velocity: %8.3f", stat.max_velocity);
	UI_ChangeTextLabel(stat_label, dbg_string);

	if (flag_stop) {
		exit(0);
	} else {
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
#if defined(LB_FULLSCREEN)
	glutFullScreen();
#endif

	UI_Init();
	stat_label = UI_CreateLabel(1, 90, 60, 5, "Label sample");
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
