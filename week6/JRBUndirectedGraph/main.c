#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "../../Libfdr/jrb.h"
#include "../../Libfdr/jval.h"



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
        jrb_insert_str(tree,strdup(v2),new_jval_i(1));
    }
    free(&node);
    free(&tree);
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


int main(){
    Graph g = make_jrb();
    FILE *fin = fopen("station.txt","r");
    char metro[4];
    int flag = 1;
    int choice;
    do{
        printf("1.Importing\n2.Show all adjacent\n3.Exit\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
                if(flag){
                        while(fscanf(fin,"%s ",metro) == 1){
                            metro[4] = '\0';
                            char staPre[4];
                            fscanf(fin,"%s ",staPre);
                            staPre[strlen(staPre)] = '\0';
                            char staCur[4];
                            for(int i = 0 ; i < 5 ; i++){
                                if(i == 4) fscanf(fin,"%s\n",staCur);
                                else fscanf(fin,"%s ",staCur);
                                staCur[strlen(staCur)]='\0';
                                addEdge(g,staPre,staCur);
                                addEdge(g,staCur,staPre);
                                strcpy(staPre,staCur);
                            }
                    }
                    flag = !flag;
                }
                break;
            case 2:
                printf("Enter the station\n");
                char sta[4];
                char **output = (char **) malloc(sizeof(char)*10*7);
                for(int i = 0 ; i < 10 ; i++) *(output + i) = (char *) malloc(sizeof(char)*7);
                scanf("%s",sta);
                int n = getAdjacentVertices(g,sta,output);
                printf("All the stations adjacent to %s station: \n",sta);
                if(n != 0){
                    for(int i = 0 ; i < n; i++){
                        printf("%s ",*(output + i));
                    }
                }else printf("none!!!\n");
                printf("\n");
                break;
        }
    }while(choice != 3);
    fclose(fin);
}