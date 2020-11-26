#include "data.h"
#include "function.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../Libfdr/dllist.h"
#include "../../Libfdr/jrb.h"
#include "../../Libfdr/jval.h"

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
    if(exs && !hasEdge(graph,v1,v2)){
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


int DAG(Graph graph){
    int output[1000],first_queue,n,outdegree_node,start;
    Dllist node,queue;
    JRB vertex;
    
    jrb_traverse(vertex,graph.vertices){
        
        JRB visited = make_jrb();
        start = jval_i(vertex->key);
        queue = new_dllist();
        dll_append(queue,new_jval_i(start));
        while(!dll_empty(queue)){
            node = dll_last(queue);
            first_queue = jval_i(node->val);
            dll_delete_node(node);
            if(!jrb_find_int(visited,first_queue)){
                jrb_insert_int(visited,first_queue,new_jval_i(1));
                n = outDegree(graph,first_queue,output);
                for(int i =0 ; i < n ;i++){
                    if(output[i] == start) return 0;
                    outdegree_node = output[i];
                    if(!jrb_find_int(visited,outdegree_node)) dll_append(queue,new_jval_i(outdegree_node));
                }
            }
        }
    }
    return 1;
}

int getAdjacentVertices(Graph graph, int vertex, int *output){
    JRB node = jrb_find_int(graph.edges,vertex);
    JRB tree = (JRB) jval_v(node->val);
    JRB temp;
    int i = 0;
    jrb_traverse(temp,tree){
        output[i++] = jval_i(temp->key);
    }
    return i;
}

void printGraph(Graph g){
    JRB node;
    jrb_traverse(node, g.edges){
        if (node != NULL){
            int output[10];
            int res = getAdjacentVertices(g, jval_i(node->key), output);
            printf("%d: ", jval_i(node->key));
            for (int i=0; i < res; i++){
                printf("%d ", output[i]);
            }
            printf("\n");
        }
    }
}