#ifndef _pQ_C_
#define _pQ_C_

#include "dllist.h"
#include <stdlib.h>
#include <stdio.h>
pQ create_list(){
    return NULL;
}

node_t make_node(int id,float cost){
    node_t new_node  = (node_t) malloc(sizeof(struct Node));
    new_node->id = id;
    new_node->cost = cost;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

void freeNode(node_t n){
    free(n);
}

int pQ_is_empty(pQ l){
    return l == NULL;
}

pQ pQ_remove(pQ l){
    pQ o = l->next;
    if(o != NULL) o->prev = NULL;
    freeNode(l);
    l = NULL;
    return o;
}
pQ pQ_insert(pQ l,node_t t){
    if(l == NULL) return t;
    pQ node = l;
    pQ prev = NULL;
    while(node != NULL){
        if(node->cost > t->cost) break;
        else { prev = node; node = node->next;}
    }

    if(node == l){
        t->next = node;
        node->prev = t;
        return t; 
    }else if(node == NULL){
        prev->next = t;
        t->prev = prev;
        return l;
    }else{
        prev->next = t;
        t->prev = prev;
        node->prev = t;
        t->next = node;
        return l;
    }
}

node_t pQ_hasNode(pQ l,int id){

    while(l != NULL){
        if( l->id == id) return l;
        else l = l->next;
    }

    return NULL;
}

node_t pQ_first(pQ l){
    return l;
}

void pQ_free_list(pQ l){
    while(l != NULL){
        pQ t = l->next;
        //free(&(l->cost));
        //free(&(l->id));
        free(l);
        l = t;
    }
}
pQ reArrangeQueue(pQ l,node_t t){
    node_t backTrack = t->prev;
    node_t lookAhead = t->next;
    while(backTrack != NULL){
        if(backTrack->cost < t->cost) break;
        else {lookAhead = backTrack;
        backTrack = backTrack->prev;}
    }
    if(t->next != NULL) t->next->prev = t->prev;
    if(t->prev != NULL) t->prev->next = t->next;

    if(backTrack == NULL){
        t->next = lookAhead;
        lookAhead->prev = t;
        return t;
    }else{
        backTrack->next = t;
        t->prev = backTrack;
        t->next = lookAhead;
        if( lookAhead != NULL) lookAhead->prev = t;
        return l;
    }
}
pQ removeAt(pQ l,node_t t){
    if(t->next != NULL) t->next->prev = t->prev;
    if(t->prev != NULL) t->prev->next = t->next;
    if(t->id == l->id ) return t->next;
    else return l;
}
#endif