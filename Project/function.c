#ifndef __FUNCTION__
#define __FUNCTION__

#include "data.h"
#include "./Libfdr/dllist.h"
#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
Graph createGraph(){
    Graph newGraph;
    newGraph.edges = make_jrb();
    newGraph.vertices = make_jrb();
    return newGraph;
}

void addEdge(Graph graph,char *start,char *destination,int distance){
    JRB node,tree;
    node = jrb_find_str(graph.edges,start);
    JRB vertex = jrb_find_str(graph.vertices,start);
    Airport *startAirport = (Airport*) jval_v(vertex->val);
    int cost = startAirport->cost;
    if(node){
        tree = (JRB) jval_v(node->val);
        jrb_insert_str(tree,destination,new_jval_i(distance* cost));
    }else{
        tree = make_jrb();
        jrb_insert_str(graph.edges,start,new_jval_v(tree));
        jrb_insert_str(tree,destination,new_jval_i(distance*cost));
    }
}

void addVertex(Graph graph,char *name, Airport *airport){
    JRB exs = jrb_find_str(graph.vertices,name);
    if(exs != NULL){
        exs->val = new_jval_v(airport);
    }else{
        jrb_insert_str(graph.vertices,strdup(name),new_jval_v(airport));
    }
    
}

void printSeparateLine(){
    printf("\n");
    for(int i = 0 ; i < 54 ; i++) printf("-");
    printf("\n");
}

void printGraph(Graph graph){
    printf("%-25s%-21s%s\n","Starting_Airport","Destination_Airport","Distance");
    JRB node;
    jrb_traverse(node,graph.edges){
        JRB subNode,tree;
        tree = (JRB)jval_v(node->val);
        printf("%-25s",jval_s(node->key));
        jrb_traverse(subNode,tree){
            printf("\n%-25s%-21s%d"," ",jval_s(subNode->key),jval_i(subNode->val));
        }
        printSeparateLine();
    }
}

Airport *getVertex(Graph graph, char *name){
    return (Airport *) jval_v(jrb_find_str(graph.vertices,name)->val);
}

int hasEdge(Graph graph,char *start,char *destination){
    JRB node = jrb_find_str(graph.edges,start);
    JRB tree;
    if(node){
        tree = (JRB) jval_v(node->val);
        JRB checkNode = jrb_find_str(tree,destination);
        if(checkNode != NULL) return 1;
    }
    return 0;
}

char *minDistanceKey(JRB tree){
    int cost = __INT_MAX__;
    char *rs;
    JRB node;
    jrb_traverse(node,tree){
        if(jval_i(node->val) < cost){
            rs = jval_s(node->key);
            cost = jval_i(node->val);
        }
    }
    return rs;
}

Dllist Dijkstra(Graph graph,char *start,char *destination){
    JRB visited = make_jrb(); // check if vertex is included/ in shortest path.
    Dllist result = new_dllist(); 
    JRB parents = make_jrb(); // store the income edge of node
    JRB queue = make_jrb();
    JRB shortestDistance = make_jrb();

    dll_append(result,new_jval_s(start));
    JRB node,tree;
    tree = (JRB) jval_v(jrb_find_str(graph.edges,start)->val);
    jrb_traverse(node,tree){
        jrb_insert_str(queue,jval_s(node->key),new_jval_i(jval_i(node->val)));
    }

    while(1){

    }
}
#endif