#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "../Libfdr/jrb.h"
#include "../Libfdr/jval.h"
#include "../Libfdr/dllist.h"
#include "queue.h"
#include "stack.h"

typedef JRB Graph;


void addEdge(Graph graph,char *v1,char *v2){
    Graph node,tree;
    node = jrb_find_str(graph,strdup(v1));
    if(node != NULL){
        tree = (JRB) jval_v(node->val);
        Graph check = jrb_find_str(tree,strdup(v2));
        if( check == NULL )
            jrb_insert_str(tree,strdup(v2),new_jval_i(1));
    }else{
        tree = make_jrb();
        jrb_insert_str(graph,strdup(v1),new_jval_v(tree));
        
        jrb_insert_str(tree,strdup(v2),new_jval_i(0));
    }
}

int getAdjacentVertices(Graph graph,char *vertex,char **output){
    Graph node = jrb_find_str(graph,vertex);
    Graph n;
    if(node == NULL) return 0;
    Graph tree = (JRB) jval_v(node->val);
    int total = 0;
    jrb_traverse(n,tree){
        strcpy( *(output+ total++),jval_s(n->key));
    }
    return total;
}

void printGraph(Graph g){
    Graph a;
    printf("%-15s%s\n","Starting Node","Destination Node");
    jrb_traverse(a,g){
        Graph k = (JRB) jval_v(a->val);
        Graph b;
        printf("%-15s",jval_s(a->key));
        jrb_traverse(b,k){
            printf("%s ",jval_s(b->key));
        }
        printf("\n");
    }
}

// void printVisited(){
//     for(int i = 0 ; i <25 ; i++) printf("%d ",visited[i]);
//     printf("\n");
// }

int checkEnd(char *s1,char *s2){
    return (strcmp(s1,s2) == 0);
}
void BFT(Graph g,char *startingNode){
    Dllist queue = new_dllist();
    JRB visited =  make_jrb();
    Graph a = jrb_find_str(g,startingNode);
    
    if(a == NULL){
        printf("There is no node like this\n");
        return;
    }
    printf("BF TRAVERSAL\n");
    printf("%s",startingNode);
    jrb_insert_str(visited,strdup(startingNode),new_jval_i(1));
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        dll_append(queue,new_jval_s(jval_s(b->key)));
        jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
    }

    while(!dll_empty(queue)){
        Dllist node = dll_first(queue);
        char *f = jval_s(node->val);
        dll_delete_node(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        printf("->%s",f);
        jrb_insert_str(visited,strdup(f),new_jval_i(1));
        jrb_traverse(b,k){
            if(jrb_find_str(visited,jval_s(b->key)) == NULL){
                dll_append(queue,new_jval_s(strdup(jval_s(b->key))));
                jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
            }
        }
    }
    printf("\n");
}

void BFS(Graph g,char *startingNode,char* destinationNode){
    Dllist queue = new_dllist();
    JRB visited =  make_jrb();
    JRB routes = make_jrb();
    Graph a = jrb_find_str(g,startingNode);
    
    if(a == NULL){
        printf("There is no node like this\n");
        return;
    }
    
    jrb_insert_str(visited,strdup(startingNode),new_jval_i(1));
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        char *f = (char*) malloc(sizeof(char)*1000);
        memset(f,'\0',sizeof(char)*1000);
        strcpy(f,startingNode);
        strcat(f," ");
        strcat(f,strdup(jval_s(b->key)));

        jrb_insert_str(routes,strdup(f),new_jval_i(1));
        dll_append(queue,new_jval_s(jval_s(b->key)));
        jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
    }

    while(!dll_empty(queue)){

        Dllist node = dll_first(queue);
        char *f = jval_s(node->val);
        dll_delete_node(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        jrb_insert_str(visited,strdup(f),new_jval_i(1));
        JRB n;

        char *curRout;
        jrb_traverse(n,routes){
            if(strstr(jval_s(n->key),f) != NULL){
                curRout = jval_s(n->key);
                break;
            }
        }

        jrb_traverse(b,k){
            if(jrb_find_str(visited,jval_s(b->key)) == NULL){
                char *g = (char*) malloc(sizeof(char)*1000);
                memset(g,'\0',sizeof(char)*1000);
                strcpy(g,curRout);
                strcat(g," ");
                strcat(g,jval_s(b->key));
                jrb_insert_str(routes,strdup(g),new_jval_i(1));
                dll_append(queue,new_jval_s(strdup(jval_s(b->key))));
                jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
            }
        }
    }
    int sizeMin = __INT_MAX__;
    char *rs = NULL;
    jrb_traverse(b,routes){
        if(strstr(jval_s(b->key),destinationNode) != NULL){
            if(strlen(jval_s(b->key)) < sizeMin){
                rs = jval_s(b->key);
                sizeMin = strlen(jval_s(b->key));
            }
        } 
    }
    if(rs != NULL) printf("%s\n",rs);
    else
        printf("There is no road from %s to %s\n",startingNode,destinationNode);
}

void DFT(Graph g,char *startingNode){
    Dllist stack = new_dllist();
    JRB visited =  make_jrb();
    Graph a = jrb_find_str(g,startingNode);
    
    if(a == NULL){
        printf("There is no node like this\n");
        return;
    }
    printf("DF TRAVERSAL\n");
    printf("%s",startingNode);
    jrb_insert_str(visited,strdup(startingNode),new_jval_i(1));
    
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        dll_append(stack,new_jval_s(jval_s(b->key)));
        jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
    }

    while(!dll_empty(stack)){
        Dllist node = dll_last(stack);
        char *f = jval_s(node->val);
        dll_delete_node(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        printf("->%s",f);
        jrb_insert_str(visited,strdup(f),new_jval_i(1));
        jrb_traverse(b,k){
            if(jrb_find_str(visited,jval_s(b->key)) == NULL){
                dll_append(stack,new_jval_s(strdup(jval_s(b->key))));
                jrb_insert_str(visited,strdup(jval_s(b->key)),new_jval_i(1));
            }
        }
    }
    printf("\n");
}

int main(){
    Graph g = make_jrb();
    FILE *fin = fopen("station.txt","r");
    char metro[4];
    char *startingNode = (char*) malloc(sizeof(char)*4);
    char *destinationNode = (char*) malloc(sizeof(char)*4);
    while(fscanf(fin,"%s %s\n",startingNode,destinationNode) == 2){
        addEdge(g,startingNode,destinationNode);
        addEdge(g,destinationNode,startingNode);
    }
    int choice;
    do{
        printf("1.Adjacent List\n2.BFT\n3.DFT\n4.Find the shortest route\n5.Exit\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                printGraph(g);
                break;
            case 2:
                printf("Enter the starting Node\n");
                fflush(stdin);
                scanf("%s",startingNode);
                BFT(g,startingNode);
                break;
            case 3:
                printf("Enter the starting Node\n");
                fflush(stdin);
                scanf("%s",startingNode);
                DFT(g,startingNode);
                break;
            case 4:
                printf("Enter the starting Node\n");
                fflush(stdin);
                scanf("%s",startingNode);
                printf("Enter the destination Node\n");
                fflush(stdin);
                scanf("%s",destinationNode);
                BFS(g,startingNode,destinationNode);
                break;
        }
    }while(choice != 5);
    fclose(fin);
}