#ifndef GRAPH_H
#define GRAPH_H

typedef void (*f_mainloop_t)(void);

int GRAPH_Init(f_mainloop_t mainloop);
void GRAPH_Redraw(const LB_Lattice_p lattice, const EXTOBJ_obj_set_p obj_set);

void GRAPH_DrawButton(lb_float x, lb_float y, lb_float width, lb_float height, char *text);
void GRAPH_DrawLabel(lb_float x, lb_float y, lb_float width, lb_float height, char *text);

#endif /* GRAPH_H */

