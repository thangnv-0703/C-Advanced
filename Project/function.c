#ifndef __FUNCTION__
#define __FUNCTION__

#include "data.h"
#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dllist.h"
Graph createGraph()
{
    Graph newGraph;
    newGraph.edges = make_jrb();
    newGraph.vertices = make_jrb();
    return newGraph;
}

void addEdge(Graph graph, int idStart, int idDestination, char *start, char *destination, float distance)
{
    JRB node, tree;
    node = jrb_find_int(graph.edges, idStart);
    JRB vertex = jrb_find_int(graph.vertices, idStart);
    if (vertex == NULL)
        return;
    Airport *startAirport = (Airport *)jval_v(vertex->val);
    float cost = startAirport->cost;
    // Create a new airport of starting airport's routes
    Airport *newAirport = (Airport *)malloc(sizeof(Airport));
    newAirport->id = idDestination;
    strcpy(newAirport->name, destination);
    newAirport->cost = distance * cost;

    if (node)
    {
        tree = (JRB)jval_v(node->val);
        if (jrb_find_int(tree, idDestination) != NULL)
            return;
        jrb_insert_int(tree, idDestination, new_jval_v(newAirport));
    }
    else
    {
        tree = make_jrb();
        jrb_insert_int(graph.edges, idStart, new_jval_v(tree));
        jrb_insert_int(tree, idDestination, new_jval_v(newAirport));
    }
}

void addVertex(Graph graph, int id, Airport *airport)
{
    JRB exs = jrb_find_int(graph.vertices, id);
    if (exs != NULL)
    {
        exs->val = new_jval_v(airport);
    }
    else
    {
        jrb_insert_int(graph.vertices, id, new_jval_v(airport));
    }
}

void printSeparateLine()
{
    printf("\n");
    for (int i = 0; i < 54; i++)
        printf("-");
    printf("\n");
}

void printGraph(Graph graph)
{
    printf("%-25s%-21s%s\n", "Starting_Airport", "Destination_Airport", "Distance");
    JRB node;
    jrb_traverse(node, graph.edges)
    {
        JRB subNode, tree;
        tree = (JRB)jval_v(node->val);
        printf("%-25s", jval_s(node->key));
        jrb_traverse(subNode, tree)
        {
            printf("\n%-25s%-21s%d", " ", jval_s(subNode->key), jval_i(subNode->val));
        }
        printSeparateLine();
    }
}

Airport *getVertex(Graph graph, char *name)
{
    return (Airport *)jval_v(jrb_find_str(graph.vertices, name)->val);
}

int hasEdge(Graph graph, int idStart, int idDestination)
{
    JRB node = jrb_find_int(graph.edges, idStart);
    JRB tree;
    if (node)
    {
        tree = (JRB)jval_v(node->val);
        JRB checkNode = jrb_find_int(tree, idDestination);
        if (checkNode != NULL)
            return 1;
    }
    return 0;
}

void printPaths(Graph graph, int *parents, int idDestination, char *s)
{
    if (parents[idDestination] == 0)
    {
        
        strcat(s, ((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name);
        
        return;
    }

    printPaths(graph, parents, parents[idDestination], s);
    strcat(s, "->");
    strcat(s, ((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name);
}

void pushToHistory(Graph graph,int *parents,int idDestination,float shortestDis){
        

}

void Dijkstra(Graph graph, int idStart, int idDestination)
{
    JRB node;
    int *parents = (int *)calloc(sizeof(int), MAXSIZE); // store the income edge of node for traceback the shortest path

    pQ queue = create_list();

    int *visited = (int *)malloc(sizeof(int) * MAXSIZE);
    for (int i = 0; i < MAXSIZE; i++)
        visited[i] = 0;

    JRB startNode = (JRB)jrb_find_int(graph.edges, idStart);
    JRB destinationNode = (JRB)jrb_find_int(graph.vertices, idDestination);

    if (startNode == NULL || destinationNode == NULL)
        return;

    visited[idStart] = 1;

    JRB tree = (JRB)jval_v(jrb_find_int(graph.edges, idStart)->val);

    jrb_traverse(node, tree)
    {
        queue = pQ_insert(queue,make_node(jval_i(node->key),((Airport*) jval_v(node->val))->cost));
        parents[jval_i(node->key)] = idStart;
    }
    float shortestDis;
    while (pQ_is_empty(queue) == 0)
    {
        int minKey = pQ_first(queue)->id;
        //printf("MinKey: %d\n",minKey);
        visited[minKey] = 1;
        shortestDis = pQ_first(queue)->cost;
       
        queue = pQ_remove(queue);

        if (minKey == idDestination)
            break;
        JRB minNodeEdges = jrb_find_int(graph.edges, minKey);
        if (minNodeEdges != NULL)
        {
            JRB outcomeMinNodeEdge = (JRB)jval_v(minNodeEdges->val);
            jrb_traverse(node, outcomeMinNodeEdge)
            {
                Airport *inforNode = (Airport *)jval_v(node->val);
                if (visited[inforNode->id] == 0)
                {
                    pQ nodeInQueue = pQ_hasNode(queue,inforNode->id);
                    if ( nodeInQueue != NULL)
                    {

                        if ( inforNode->cost + shortestDis < nodeInQueue->cost)
                        {
                            nodeInQueue->cost = inforNode->cost + shortestDis;
                            parents[inforNode->id] = minKey;
                        }
                    }else
                    {
                    queue = pQ_insert(queue,make_node(inforNode->id,shortestDis + inforNode->cost));
                    parents[inforNode->id] = minKey;
                    }
                }
                

            }

            
        }
        
    }

    if(pQ_is_empty(queue)){
        //printf("No route between two airports\n");
    }else{
        //printf("Begin:\n");
        FILE *out = fopen("history.dat","a+");
        char *s = (char*) malloc(sizeof(char)*2000);
        memset(s,'\0',sizeof(char)*2000);
        printPaths(graph,parents,idDestination,s);
        for(int i = 0 ; i  < MAXSIZE ; i++){
            if(parents[i] != 0) printf("%d\n",parents[i]);
        }
        printPaths(graph,parents,idDestination,s);
        exit(1);
        //printf("\nCost: %f\n",shortestDis);
        //fprintf(out,"%f ",shortestDis);
        
        //fputs(s,out);
        
        //fputs("\n",out);
    
        //free(s);
        //printf("End\n");
        fclose(out);
        //pushToHistory(graph,parents,idDestination,shortestDis);
    }
    
    pQ_free_list(queue);
    //free(costTable);
    free(visited);
    free(parents);
}
#endif