#ifndef Fucntion
#define Function

#include "data.h"

Graph createGraph(int sizemax);
void addEdge(Graph graph,int v1,int v2);
int adjacent(Graph graph,int v1,int v2);
int getAdjacentVertices(Graph graph,int vertex,int *output);
void dropGraph(Graph graph);
void printGraph(Graph Graph);
#endif