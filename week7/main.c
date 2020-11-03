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

void BFS(Graph g,char *startingNode,char* destinationNode){
    // char *roadMap = (char*) malloc(sizeof(char)*1000);
    // memset(roadMap,'\0',sizeof(char)*1000);
    queue node = createQueue();
    Graph a = jrb_find_str(g,startingNode);
    if(a == NULL){
        printf("There is no node like this\n");
        return;
    }
    
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        node = enqueue(node,jval_s(b->key));
        visited[jval_s(b->key)[indexData] - lower] = 1;
    }
    //printf("BFS TRAVERSAL\n");
    printf("%s",startingNode);

    while(!isEmpty(node)){
        char *f = peek(node)->nodePos;
        node = dequeue(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        if(a == NULL) continue;
        visited[(int) f[indexData] - lower] = 1;
        jrb_traverse(b,k){
            int index = (int) jval_s(b->key)[indexData] - lower;
            if(visited[index] == 0){
                node = enqueue(node,jval_s(b->key));
                visited[index] = 1;
            }
        }
        printf("->%s",f);
        if(strcmp(f,destinationNode) == 0){
            memset(visited,0,sizeof(int)*25);
            printf("\n");
            return;
        }
    }
    printf("There is no road from %s to %s\n",startingNode,destinationNode);
    memset(visited,0,sizeof(int)*25);
}

void DFS(Graph g,char *startingNode,char *destinationNode){
    int indexData = 1;
    stack node = create_stack();
    int lower = 65;
    visited[(int) startingNode[indexData] - lower] = 1;
    Graph a = jrb_find_str(g,startingNode);
    if(a==NULL){
        printf("\nThere is no node like this\n");
        return;
    }
    
    Graph b,k;
    k = (JRB) jval_v(a->val);
    jrb_traverse(b,k){
        node = push(node,jval_s(b->key));
        visited[jval_s(b->key)[indexData] - lower] = 1;
    }
    printf("BFS TRAVERSAL\n");
    printf("%s",startingNode);

    while(!is_empty(node)){
        char *f = top(node);
        node = pop(node);
        a = jrb_find_str(g,f);
        k = (JRB) jval_v(a->val);
        if(a == NULL) continue;
        visited[(int) f[indexData] - lower] = 1;
        jrb_traverse(b,k){
            int index = (int) jval_s(b->key)[indexData] - lower;
            if(visited[index] == 0){
                node = push(node,jval_s(b->key));
                visited[index] = 1;
            }
        }
        printf("->%s",f);
        if(strcmp(f,destinationNode) == 0){
            memset(visited,0,sizeof(int)*25);
            printf("\n");
            return;
        }
    }
    printf("\nThere is no road from %s to %s\n",startingNode,destinationNode);
    memset(visited,0,sizeof(int)*25);
}

int main(){
    Graph g = make_jrb();
    FILE *fin = fopen("text.txt","r");
    char metro[4];
    memset(visited,0,sizeof(int)*25);
    while(fscanf(fin,"%s ",metro) == 1){
        metro[4] = '\0';
        char *staPre = (char*) malloc(sizeof(char)*4);
        fscanf(fin,"%s ",staPre);
        staPre[strlen(staPre)] = '\0';
        char *staCur = (char *) malloc(sizeof(char)*4);
        for(int i = 0 ; i < 9 ; i++){
            if(i == 9) fscanf(fin,"%s\n",staCur);
            else fscanf(fin,"%s ",staCur);
            staCur[strlen(staCur)]='\0';
            addEdge(g,staPre,staCur);
            addEdge(g,staCur,staPre);
            strcpy(staPre,staCur);
        }
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
                char *destinationNode = (char*) malloc(sizeof(char));
                fflush(stdin);
                scanf("%s",startingNode);
                printf("Enter the destination Node\n");
                fflush(stdin);
                scanf("%s",destinationNode);
                BFS(g,metro,destinationNode);
                break;
            case 3:
                printf("Enter the starting Node\n");
                fflush(stdin);
                scanf("%s",startingNode);
                printf("Enter the destination Node\n");
                fflush(stdin);
                scanf("%s",destinationNode);
                DFS(g,startingNode,destinationNode);
                break;
        }
    }while(choice != 4);
    fclose(fin);
}