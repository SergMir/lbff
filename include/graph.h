#ifndef GRAPH_H
#define GRAPH_H

typedef void (*f_mainloop_t)(void);

int GRAPH_Init(f_mainloop_t mainloop);
void GRAPH_Redraw(const LB_Lattice_p lattice, const EXTOBJ_obj_p objects, uint objcnt);

#endif /* GRAPH_H */

