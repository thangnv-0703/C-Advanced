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

void addVertex(Graph graph, int id, char* name){
    JRB exs = jrb_find_int(graph.vertices,id);
    if (exs)
        exs->val.s = name;
    else
        jrb_insert_int(graph.vertices,id,new_jval_s(strdup(name)));
    
}

char *getVertex(Graph graph, int id){
    return jval_s(jrb_find_int(graph.vertices,id)->val);
}
void addEdge(Graph graph, int v1, int v2){
    JRB exs = jrb_find_int(graph.edges,v1);
    if(exs){
        JRB jrbValueExs = (JRB) jval_v(exs->val);
        JRB subExs = jrb_find_int(jrbValueExs,v2);
        if(subExs == NULL)
            jrb_insert_int(jrbValueExs,v2,new_jval_i(1));
        else
            return;
    }else{
        JRB tree;
        tree = make_jrb();
        jrb_insert_int(graph.edges,v1,new_jval_v(tree));
        jrb_insert_int(tree,v2,new_jval_i(1));
    }
}