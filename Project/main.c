#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "function.h"
#include "time.h"
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
        name[strlen(name) - 1] = '\0';
        Airport *newAirport = (Airport*) malloc(sizeof(Airport));
        newAirport->id = id;
        strcpy(newAirport->name,name);
        newAirport->cost = 1;
        addVertex(graph,id,newAirport);
    }
    float lat1,lat2,lon1,lon2;
    int idSource,idDes;
    char sourceAir[100],DesAir[100];
    float distance;
    fclose(ptr);
    while(!feof(ptr2)){
        fscanf(ptr2,"%s %d %s %d %f %f %f %f %f\n",sourceAir,&idSource,DesAir,&idDes,&lat1,&lon1,&lat2,&lon2,&distance);
        JRB findingNode = jrb_find_int(graph.vertices,idSource);
        if(idSource == idDes) continue;
        Airport *inforNode;
        if(findingNode != NULL) inforNode = (Airport*) jval_v(findingNode->val);
        if(inforNode != NULL) strcpy(sourceAir,inforNode->name);
        findingNode = jrb_find_int(graph.vertices,idDes);
        if(findingNode != NULL) inforNode = (Airport*) jval_v(findingNode->val);
        if(inforNode != NULL) strcpy(DesAir,inforNode->name);
        addEdge(graph,idSource,idDes,sourceAir,DesAir,distance);
    }
    fclose(ptr2);
    return graph;
}

int random(int minN, int maxN){
    return minN + rand() % (maxN + 1 - minN);
}

int main(){

    Graph graph = input();
    JRB node;
    int *arr = (int *) malloc(sizeof(int)*7699);
    int p = 0;
    jrb_traverse(node,graph.vertices){
        *(arr + p) = jval_i(node->key);
         //printf("%d\n",jval_i(node->key));
         p +=1;
    }
    srand((int)time(0));
    int r;
    clock_t t1,t2;
    t1 = clock();

    for(int i = 0; i < 100000; i++){
        int j = random(0,7698);
        int k = random(0,7698);
        //printf("%d %d\n",j,k);
        //Dijkstra(graph,3199,i);
        Dijkstra(graph,arr[j],arr[k]);
    }
    //Dijkstra(graph,2397,5630);
    //Dijkstra(graph,1,2000);
    t1 = clock() - t1;
    printf("%f\n",(double) t1/CLOCKS_PER_SEC);
    printf("hiiii\n");
}