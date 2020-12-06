#ifndef __FUNCTION__
#define __FUNCTION__
#include "data.h"
Graph createGraph();
void addVertex(Graph graph,char *name, Airport *airport);
Airport *getVertex(Graph graph, char *name);
void addEdge(Graph graph,char *start,char *destination,int distance);
void printGraph(Graph graph);
Dllist Dijkstra(Graph graph,char *start,char *destination);
int hasEdge(Graph graph,char *start,char *destination);
#endif