#ifndef Fucntion
#define Function

#include "data.h"

Graph createGraph();
void addVertex(Graph graph, int id, char* name);
char *getVertex(Graph graph, int id);
void addEdge(Graph graph, int v1, int v2);
int hasEdge(Graph graph, int v1, int v2);
int inDegree(Graph graph, int v, int* output);
int outDegree(Graph graph, int v, int* output);
int DAG(Graph graph);
void dropGraph(Graph graph);
#endif