#ifndef __FUNCTION__
#define __FUNCTION__

#include "data.h"
#include "../Libfdr/dllist.h"
#include "../Libfdr/jrb.h"
#include "../Libfdr/jval.h"
#include "stdlib.h"
typedef JRB Graph;

void addEdge(Graph graph,char *start,char *destination){
    Graph node,tree;
    node = jrb_find_str(graph,strdup(start));
    if(node != NULL){
        tree = (JRB) jval_v(node->val);
        Graph check = jrb_find_str(tree,strdup(destination));
        if( check == NULL )
            jrb_insert_str(tree,strdup(destination),new_jval_v(1));
        else{

        }
    }else{
        tree = make_jrb();
        jrb_insert_str(graph,strdup(start),new_jval_v(tree));
        jrb_insert_str(tree,strdup(destination),new_jval_i(1));
    }
}
#endif