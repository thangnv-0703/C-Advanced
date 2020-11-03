#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "../Libfdr/jrb.h"
#include "../Libfdr/jval.h"
#include "queue.h"
#include "stack.h"

typedef JRB Graph;
int visited[26];

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
        jrb_insert_str(tree,strdup(v2),new_jval_i(1));
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

void BFS(Graph g,char *startingNode){
    queue node = createQueue();
    int lower = 65;
    visited[(int) startingNode[0] - lower] = 1;
    Graph a = jrb_find_str(g,startingNode);
    if(a==NULL){
        printf("There is no node like this\n");
        return;
    }
    
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        node = enqueue(node,jval_s(b->key));
        visited[jval_s(b->key)[0] - lower] = 1;
    }
    printf("BFS TRAVERSAL\n");
    printf("%s",startingNode);

    while(!isEmpty(node)){
        char *f = peek(node)->nodePos;
        node = dequeue(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        if(a == NULL) continue;
        visited[(int) f[0] - lower] = 1;
        jrb_traverse(b,k){
            int index = (int) jval_s(b->key)[0] - lower;
            if(visited[index] == 0){
                node = enqueue(node,jval_s(b->key));
                visited[index] = 1;
            }
        }
        printf("->%s",f);
    }
    printf("\n");
    memset(visited,0,sizeof(int)*25);
}

void DFS(Graph g,char *startingNode){
    stack node = create_stack();
    int lower = 65;
    visited[(int) startingNode[0] - lower] = 1;
    Graph a = jrb_find_str(g,startingNode);
    if(a==NULL){
        printf("There is no node like this\n");
        return;
    }
    
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        node = push(node,jval_s(b->key));
        visited[jval_s(b->key)[0] - lower] = 1;
    }
    printf("BFS TRAVERSAL\n");
    printf("%s",startingNode);

    while(!is_empty(node)){
        char *f = top(node);
        node = pop(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        if(a == NULL) continue;
        visited[(int) f[0] - lower] = 1;
        jrb_traverse(b,k){
            int index = (int) jval_s(b->key)[0] - lower;
            if(visited[index] == 0){
                node = push(node,jval_s(b->key));
                visited[index] = 1;
            }
        }
        printf("->%s",f);
    }
    printf("\n");
    memset(visited,0,sizeof(int)*25);
}

int main(){
    Graph g = make_jrb();
    FILE *fin = fopen("text.txt","r");
    char *start = (char*) malloc(sizeof(char));
    char *end = (char*) malloc(sizeof(char));
    memset(start,'\0',sizeof(char));
    memset(end,'\0',sizeof(char));
    memset(visited,0,sizeof(int)*26);
    while(fscanf(fin,"%s %s\n",start,end) == 2){
        addEdge(g,strdup(start),strdup(end));
        addEdge(g,strdup(end),strdup(start));
    }
    int choice;
    do{
        printf("1.Adjacent List\n2.BFS\n3.DFS\n4.Exit\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                printGraph(g);
                break;
            case 2:
                printf("Enter the starting Node\n");
                char *startingNode = (char*) malloc(sizeof(char));
                fflush(stdin);
                scanf("%s",start);
                BFS(g,start);
                break;
            case 3:
                printf("Enter the starting Node\n");
                fflush(stdin);
                scanf("%s",start);
                DFS(g,start);
                break;
        }
    }while(choice != 4);
}