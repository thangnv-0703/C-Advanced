#ifndef __FUNCTION__
#define __FUNCTION__
#include "data.h"
Graph createGraph();
void addVertex(Graph graph,char *name, Airport *airport);
char *getVertex(Graph graph, int id);
void addEdge(Graph graph,char *start,char *destination,int distance);

#endif