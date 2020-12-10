#ifndef __FUNCTION__
#define __FUNCTION__

#include "data.h"
#include "./Libfdr/dllist.h"
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

char *minDistanceKey(JRB tree)
{
    int cost = __INT_MAX__ * 1.0;
    char *rs;
    JRB node;
    jrb_traverse(node, tree)
    {
        Airport *a = jval_v(node->val);
        if (a->cost < cost)
        {
            rs = jval_s(node->key);
            cost = a->cost;
        }
    }
    return rs;
}

int emptyQueue(JRB queue)
{
    JRB node;
    jrb_traverse(node, queue)
    {
        return 0;
    }
    return 1;
}

Airport *duplicateInforAirport(Airport *a)
{
    Airport *b = (Airport *)malloc(sizeof(Airport));
    strcpy(b->name, a->name);
    b->cost = a->cost;
    b->id = a->id;
    return b;
}

void printPaths(JRB parents, char *destination, int count)
{
    JRB node = jrb_find_str(parents, destination);
    if (node == NULL)
    {
        printf("%s", destination);
        return;
    }
    printPaths(parents, jval_s(node->val), count + 1);
    printf("->%s", destination);
}

Dllist Dijkstra(Graph graph, int idStart, int idDestination)
{
    JRB visited = make_jrb(); // check if vertex is included/ in shortest path.
    JRB parents = make_jrb(); // store the income edge of node for traceback the shortest path
    JRB queue = make_jrb();   // store the current nodes

    // Initally store information of startNode
    JRB node, tree;
    JRB startNode = (JRB)jrb_find_int(graph.edges, idStart);
    JRB destinationNode = (JRB)jrb_find_int(graph.vertices, idDestination);
    if (startNode == NULL || destinationNode == NULL)
        return NULL;
    JRB inforStartNode = jrb_find_int(graph.vertices, idStart);
    Airport *inforStartAirport = (Airport *)jval_v(inforStartNode->val);
    Airport *inforDestinationAirport = (Airport *)jval_v(destinationNode->val);

    jrb_insert_str(visited, inforStartAirport->name, new_jval_i(0));
    tree = (JRB)jval_v(startNode->val);
    // Store the information of outcome edge of startNode
    jrb_traverse(node, tree)
    {
        Airport *a = (Airport *)jval_v(node->val);
        Airport *b = duplicateInforAirport(a);
        jrb_insert_str(queue, strdup(a->name), new_jval_v(b));
        jrb_insert_str(parents, strdup(a->name), new_jval_s(strdup(inforStartAirport->name)));
    }

    float shortestCost;
    while (!emptyQueue(queue))
    {
        // if (jrb_first(queue) == NULL)
        //     printf("NULL\n");
        // printf("Queue:\n");
        // jrb_traverse(node,queue){
        //     Airport *p = jval_v(node->val);
        //     printf("%d %s %f\n",p->id,p->name,p->cost);
        // }

        // printf("\n");
        // printf("Visited:\n");
        // jrb_traverse(node,visited){
        //     printf("%s\n",jval_s(node->key));
        // }
        // printf("\n");
        
        char *minValue = minDistanceKey(queue);
        //printf("Min value: %s\n",minValue);
        //printf("%s %s\n",minValue,inforDestinationAirport->name);
        JRB minNodeQueue = jrb_find_str(queue, minValue);
        //printf("%s %s\n",inforDestinationAirport->name,minValue);
        Airport *minAirport = (Airport *)jval_v(minNodeQueue->val);
        jrb_insert_str(visited, strdup(minValue), new_jval_i(1));

        float minNodeDist = minAirport->cost;
        shortestCost = minNodeDist;
        jrb_delete_node(minNodeQueue);

        if (strcmp(inforDestinationAirport->name, minValue) == 0)
            break;

        JRB minNodeEdges = jrb_find_int(graph.edges, minAirport->id);
        JRB outcomesMinNodeEdges;

        if (minNodeEdges != NULL)
        {

            outcomesMinNodeEdges = (JRB)jval_v(minNodeEdges->val);
            //printf("OutCome Node:\n");
            jrb_traverse(node, outcomesMinNodeEdges)
            {
                Airport *airportNode = (Airport *)jval_v(node->val);
                JRB nodeQueue = jrb_find_str(queue, airportNode->name);
                //printf("%d %s %f\n",airportNode->id,airportNode->name,airportNode->cost);

                Airport *t = duplicateInforAirport(airportNode);

                t->cost += minNodeDist;
                if (jrb_find_str(visited, airportNode->name) == NULL)
                {
                    if (nodeQueue == NULL)
                    {
                        if(jrb_find_str(parents,airportNode->name) == NULL) jrb_insert_str(parents, strdup(airportNode->name), new_jval_s(strdup(minValue)));
                        jrb_insert_str(queue, strdup(airportNode->name), new_jval_v(t));
                    }
                    else
                    {
                        Airport *nodeQueueVal = (Airport *)jval_v(nodeQueue->val);
                        if (t->cost < nodeQueueVal->cost)
                        {
                            nodeQueueVal->cost = t->cost;
                            JRB nodeInParentsTree = jrb_find_str(parents, airportNode->name);
                            char *new_parents = strdup(minValue);
                            char *temp = strdup(airportNode->name);
                            jrb_delete_node(nodeInParentsTree);
                            jrb_insert_str(parents,temp,new_jval_s(new_parents));
                            
                        }
                    }
                }
            }
            
        }
        //printf("%s\n",minValue);
        //printf("%f\n",shortestCost);
    }
    if (emptyQueue(queue))
        printf("No path between two airports\n");
    else
    {
        // int i = 0;
        // jrb_traverse(node,visited){
        //     i++;
        //     printf("%s %d\n",jval_s(node->key),strlen(jval_s(node->key)));
        // }
        // printf("%d\n",i);
        // jrb_traverse(node,parents){
        //     printf("%s %s\n",jval_s(node->key),jval_s(node->val));
        // }
        //printPaths(parents, inforDestinationAirport->name, 0);
        //printf("\nCost: %f\n", shortestCost);
    }
}
#endif