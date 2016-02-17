#ifndef __GRAPH_H__
#define __GRAPH_H__

#ifndef DEFAULT_GRAPH_WIDTH
#define DEFAULT_GRAPH_WIDTH 60
#endif

#ifndef DEFAULT_GRAPH_HEIGHT
#define DEFAULT_GRAPH_HEIGHT 20
#endif

void print_graph(double * values, int pointCount);
void print_graph_at_size(double * values, int pointCount, int width, int height);

#endif
