#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "function.h"
#include "./Libfdr/dllist.h"
#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"

float float_rand( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

Graph input(){
    Graph graph = createGraph();
    FILE *ptr = fopen("airports.dat","r+");
    FILE *ptr2 = fopen("routes.dat","r+");
    int id;
    char *name = (char*) malloc(sizeof(char)*100);
    memset(name,'\0',sizeof(name));
    while(!feof(ptr)){
        fscanf(ptr,"%d ",&id);
        fgets(name,100,ptr);
        Airport *newAirport = (Airport*) malloc(sizeof(Airport));
        newAirport->id = id;
        strcpy(newAirport->name,name);
        newAirport->cost = 1;
        addVertex(graph,id,newAirport);
    }
    float lat1,lat2,lon1,lon2;
    int idSource,idDes;
    char sourceAir[10],DesAir[10];
    float distance;
    fclose(ptr);
    while(!feof(ptr2)){
        fscanf(ptr2,"%s %d %s %d %f %f %f %f %f\n",sourceAir,&idSource,DesAir,&idDes,&lat1,&lon1,&lat2,&lon2,&distance);
        addEdge(graph,idSource,idDes,distance);
    }
    fclose(ptr2);
    return graph;
}

int main(){
    Graph graph = input();
    JRB node;

}