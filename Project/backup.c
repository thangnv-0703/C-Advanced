#ifndef __FUNCTION__
#define __FUNCTION__

#include "data.h"
#include "./Libfdr/jrb.h"
#include "./Libfdr/jval.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

int minDistanceKey(float *queue)
{
    float *cost = (float *)calloc(sizeof(float), 1);
    (*cost) = 9999999.0;
    int rs;
    int *i = (int *)malloc(sizeof(int));
    (*i) = 0;
    for (; (*i) < MAXSIZE; (*i) += 1)
    {
        if (queue[(*i)] < (*cost) && queue[(*i)] != 0){
            (*cost) = queue[(*i)];
            rs = (*i);
        }
    }
    free(i);
    return rs;
}

int emptyQueue(float *queue)
{
    int *i = (int*) malloc(sizeof(int));
    (*i) = 0;
    for (; (*i) < MAXSIZE; (*i) += 1)
    {
        if (queue[(*i)] != 0 )
            return 0;
    }
    return 1;
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

void Dijkstra(Graph graph, int idStart, int idDestination)
{
    JRB node;
    int *parents = (int *)calloc(sizeof(int), MAXSIZE); // store the income edge of node for traceback the shortest path

    float *queue = (float *)calloc(sizeof(float), MAXSIZE);
    jrb_traverse(node, graph.vertices)
    {
        queue[jval_i(node->key)] = 0;
    }

    // float *costTable = (float *)calloc(sizeof(float), MAXSIZE * MAXSIZE);
    // jrb_traverse(node, graph.edges)
    // {
    //     JRB subNode;
    //     JRB tree = (JRB)jval_v(node->val);
    //     jrb_traverse(subNode, tree)
    //     {
    //         costTable[jval_i(node->key) * MAXSIZE + jval_i(subNode->key)] = ((Airport *)jval_v(subNode->val))->cost;
    //     }
    // }

    int *visited = (int *)malloc(sizeof(int) * MAXSIZE);
    for (int i = 0; i < MAXSIZE; i++)
        visited[i] = 0;

    // for(int i = 0 ; i < MAXSIZE ; i++){
    //     for(int j = 0 ; j < MAXSIZE ; j++){
    //         if(costTable[i*MAXSIZE + j] != 0){
    //             printf("%f ",costTable[i*MAXSIZE + j]);
    //         }
    //     }
    //     printf("\n");
    // }

    JRB startNode = (JRB)jrb_find_int(graph.edges, idStart);
    JRB destinationNode = (JRB)jrb_find_int(graph.vertices, idDestination);

    if (startNode == NULL || destinationNode == NULL)
        return;

    visited[idStart] = 1;

    JRB tree = (JRB)jval_v(jrb_find_int(graph.edges, idStart)->val);

    jrb_traverse(node, tree)
    {
        //printf("%f\n",costTable[idStart*MAXSIZE + jval_i(node->key)]);
        queue[jval_i(node->key)] = ((Airport*) jval_v(node->val))->cost;
        parents[jval_i(node->key)] = idStart;
    }
    float shortestDis;
    while (emptyQueue(queue) == 0)
    {
        int minKey = minDistanceKey(queue);
        // printf("%d\n",minKey);
        // for(int i = 0 ; i < MAXSIZE ; i++) if(queue[i] != 0) printf("%f\n",queue[i]);
        // printf("\n");
        //JRB minNodeQueue = jrb_find_int(queue,minKey);
        visited[minKey] = 1;
        shortestDis = queue[minKey];
       
        //printf("%d\n",minKey);
        queue[minKey] = 0;

        if (minKey == idDestination)
            break;
        JRB minNodeEdges = jrb_find_int(graph.edges, minKey);
        if (minNodeEdges != NULL)
        {
            JRB outcomeMinNodeEdge = (JRB)jval_v(minNodeEdges->val);
            jrb_traverse(node, outcomeMinNodeEdge)
            {
                Airport *inforNode = (Airport *)jval_v(node->val);
                //JRB nodeInQueue = jrb_find_int(queue,inforNode->id);

                if (visited[inforNode->id] == 0)
                {
                    if (queue[inforNode->id] != 0)
                    {

                        if ( inforNode->cost + shortestDis < queue[inforNode->id])
                        {
                            queue[inforNode->id] = inforNode->cost + shortestDis;
                            parents[inforNode->id] = minKey;
                        }
                    }else
                    {
                    queue[inforNode->id] = shortestDis + inforNode->cost;
                    parents[inforNode->id ] = minKey;
                    }
                }
                

            }

            
        }
        
    }

    // // jrb_traverse(node,parents){
    // //     printf("%d %d\n",jval_i(node->key),jval_i(node->val));
    // // }
    
    if(emptyQueue(queue)){
        //printf("No route between two airports\n");
    }else{
        // FILE *out = fopen("history.dat","a+");
        // char *s = (char*) malloc(sizeof(char)*1000);
        // memset(s,'\0',sizeof(char)*1000);
        // printPaths(graph,parents,idDestination,s);
        // //printf("\nCost: %f\n",shortestDis);
        // fputs("\n",out);
        // fprintf(out,"%f ",shortestDis);
        // fputs(s,out);
        // free(s);
        // fclose(out);
    }
    free(queue);
    //free(costTable);
    free(visited);
    free(parents);
}
#endif