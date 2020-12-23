#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "data.h"
#include "function.h"

int main(){
    
    int i, n, output[10];
    Graph g = createGraph();
    // addVertex(g, 0, "V0");
    // addVertex(g, 1, "V1");
    // addVertex(g, 2, "V2");
    // addVertex(g, 3, "V3");
    // addEdge(g, 0, 1);
    // addEdge(g, 0, 2);
    // addEdge(g, 1, 2);
    // addEdge(g, 1, 3);
    // addEdge(g,3,1);
    for(int j = 0 ; j < 100000 ; j++){
        addVertex(g,j," ");
        addEdge(g,0,j);
    }
    printf("hi\n");
    //printGraph(g);
    // if (DAG(g))
    // printf("The graph is acycle\n");
    //     else
    // printf("Have cycles in the graph\n");

}