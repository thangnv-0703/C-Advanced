#include "data.h"
#include "function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Graph createGraph(int sizemax){
    Graph newGraph;
    newGraph.edges = make_jrb();
    newGraph.vertices = make_jrb();
    return newGraph;
}

void addVertices(Graph graph,int v1,int v2){
    
}
int adjacent(Graph graph,int v1,int v2){
   
}
int getAdjacentVertices(Graph graph,int vertex,int *output){
  
}
void dropGraph(Graph graph){
    
}
void printGraph(Graph graph){
    
}