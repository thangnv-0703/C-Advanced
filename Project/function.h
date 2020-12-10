#ifndef __FUNCTION__
#define __FUNCTION__
#include "data.h"
#include "./Libfdr/dllist.h"
Graph createGraph();
void addVertex(Graph graph,int id, Airport *airport);
Airport *getVertex(Graph graph, char *name);
void addEdge(Graph graph,int idStart,int idDestination,char *start,char *destination,float distance);
void printGraph(Graph graph);
Dllist Dijkstra(Graph graph,int idStart,int idDestination);
int hasEdge(Graph graph,int idStart,int idDestination);
#endif