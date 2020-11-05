#include "data.h"
#include "function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Graph createGraph(int sizemax){
    Graph newGraph;
    newGraph.sizemax = sizemax;
    newGraph.matrix = (int*) malloc(sizeof(int)*sizemax*sizemax);
    memset(newGraph.matrix,0,sizeof(int)*sizemax*sizemax);
    return newGraph;
}

void addEdge(Graph graph,int v1,int v2){
    graph.matrix[v1*graph.sizemax+v2] = 1;
    graph.matrix[v2*graph.sizemax+v1] = 1;
}
int adjacent(Graph graph,int v1,int v2){
    return graph.matrix[v1*graph.sizemax + v2];
}
int getAdjacentVertices(Graph graph,int vertex,int *output){
    int n = 0;
    for(int i = 0 ; i < graph.sizemax ; i++){
        if(graph.matrix[vertex*graph.sizemax + i]){
            *(output + n++) = vertex*graph.sizemax + i;
        }
    }
    return n;
}
void dropGraph(Graph graph){
    for(int i = 0 ; i < graph.sizemax*graph.sizemax ; i++)
        free(graph.matrix + i);
}
void printGraph(Graph graph){
    for(int i = 0 ; i < graph.sizemax ; i++){
        for(int j = 0 ; j < graph.sizemax ; j++)
            printf("%d ",*(graph.matrix + (i*graph.sizemax + j)));
        printf("\n");
    }
}