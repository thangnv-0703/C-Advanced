void printGraph(Graph graph)
{
    printf("%-40s%-80s%s\n", "Starting_Airport", "Destination_Airport", "Distance");
    JRB node;
    jrb_traverse(node, graph.edges)
    {
        JRB subNode, tree;
        tree = (JRB)jval_v(node->val);

        printf("%-40s", ((Airport*) jval_v(jrb_find_int(graph.vertices,jval_i(node->key))->val))->name );
        jrb_traverse(subNode, tree)
        {
            printf("\n%-40s%-80s%f", " ", ((Airport*) jval_v(subNode->val))->name, ((Airport*) jval_v(subNode->val))->cost);
        }
        printSeparateLine();
    }
}