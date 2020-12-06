#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "function.h"
#include "./Libfdr/dllist.h"
#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"


int main(){
    Graph city = createGraph();
    Airport *c1 = (Airport*) malloc(sizeof(Airport));
    Airport *c2 = (Airport*) malloc(sizeof(Airport));
    c1->cost = 20;
    strcpy(c1->name,"HaNoi");
    c1->id = 1;
    c2->cost = 10;
    strcpy(c2->name,"HCM");
    c2->id = 2;
    addVertex(city,"HaNoi",c1);
    addVertex(city,"HCM",c2);
    addEdge(city,"HaNoi","HCM",2000);
    addEdge(city,"HCM","HaNoi",200);
    printf("%d\n",hasEdge(city,"HCM","HaNoi"));
}