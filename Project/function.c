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
        jrb_insert_str(graph.vertices,start,new_jval_v(tree));
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


#endif