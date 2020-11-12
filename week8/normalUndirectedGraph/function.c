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

int hasEdge(Graph graph, int v1, int v2){
    JRB node = jrb_find_int(graph.edges,v1);
    JRB valTree = (JRB) jval_v(node->val);
    if(valTree){
        JRB trv;
        jrb_traverse(trv,valTree){
            if( trv->key.i == v2){
                return 1;
            }
        }
    }
    return 0;
}
int inDegree(Graph graph, int v, int* output){
    JRB node,valNode;
    int n = 0;
    jrb_traverse(node,graph.edges){
        JRB tree = (JRB) jval_v(node->val);
        valNode = jrb_find_int(tree,v);
        if(valNode){
            *(output+n++) = jval_i(valNode->key);
        }
    }
    return n;
}

int outDegree(Graph graph, int v, int* output){
    JRB node,valNode;
    int n = 0;
    node = jrb_find_int(graph.edges,v);
    if(node){
        JRB tree = (JRB) jval_s(node->val);
        jrb_traverse(valNode,tree){
            *(output + n++) = jval_i(valNode->key);
        }
    }
    return n;
}