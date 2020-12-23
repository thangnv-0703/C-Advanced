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
    // Create a new airport of starting airport's routes
    Airport *newAirport = (Airport *)malloc(sizeof(Airport));
    newAirport->id = idDestination;
    strcpy(newAirport->name, destination);
    newAirport->cost = distance * startAirport->cost;

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
    node = NULL; free(node);
    tree = NULL; free(tree);
    vertex = NULL; free(vertex);
    startAirport = NULL; free(startAirport);
    //newAirport = NULL; free(newAirport);

}

void addVertex(Graph graph, int id, Airport *airport)
{
    JRB exs = jrb_find_int(graph.vertices, id);
    if (exs != NULL)
    {
        exs->val = new_jval_v(airport);
        exs = NULL;
        free(exs);
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

void printPaths(Graph graph, int *parents, int idDestination,FILE *ptr)
{
    if (parents[idDestination] == 0)
    {
        fputs(((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name,ptr);
        //printf("%s",((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name);
        return;
    }

    printPaths(graph, parents, parents[idDestination], ptr);
    fputs("->",ptr);
    //printf("->");
    //printf("%s",((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name);
    fputs(((Airport *)jval_v(jrb_find_int(graph.vertices, idDestination)->val))->name,ptr);
}


void Dijkstra(Graph graph, int idStart, int idDestination)
{
    JRB node;
    int *parents = (int *)calloc(sizeof(int), MAXSIZE); // store the income edge of node for traceback the shortest path
    for(int i = 0 ; i < MAXSIZE ; i++) parents[i] = 0;
    pQ queue = create_list();

    int *visited = (int *)malloc(sizeof(int) * MAXSIZE);
    int *i = (int *) malloc(sizeof(int));
    (*i) = 0;
    for (; (*i) < MAXSIZE; (*i)+=1)
        visited[(*i)] = 0;
    free(i);

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
    tree = NULL;
    free(tree);
    float shortestDis;
    int count = 0 ;
    while (pQ_is_empty(queue) == 0)
    {
        int minKey = pQ_first(queue)->id;
        //printf("%d\n",minKey);
        visited[minKey] = 1;
        shortestDis = pQ_first(queue)->cost;
        pQ l = queue;
        
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
                            //queue = reArrangeQueue(queue,nodeInQueue);
                            queue = removeAt(queue,nodeInQueue);
                            queue = pQ_insert(queue,make_node(inforNode->id,inforNode->cost + shortestDis));
                            parents[inforNode->id] = minKey;
                        }
                    }else{
                        queue = pQ_insert(queue,make_node(inforNode->id,shortestDis + inforNode->cost));
                        parents[inforNode->id] = minKey;
                    }
                }
            }
        }
    }

    if(pQ_is_empty(queue)){
        //printf("Noooo\n");
    }else{
        FILE *out = fopen("history.dat","a+");
        fprintf(out,"%f ",shortestDis);
        printPaths(graph,parents,idDestination,out);
        fputs("\n",out);
        free(out);
        fclose(out);
    }

    pQ_free_list(queue);
    free(visited);
    free(parents);
}
#endif