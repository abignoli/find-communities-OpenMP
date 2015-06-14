#ifndef DYNAMIC_GRAPH_H
#define DYNAMIC_GRAPH_H

//DynamicGraph

typedef struct edge {
	int dest;
} edge;

typedef struct dynamic_edge_array{
	edge *addr;
	int self_loop;
	int count;
	int size;
} dynamic_edge_array;

int dynamic_edge_array_init(dynamic_edge_array *da, int initSize);

int dynamic_edge_array_insert(dynamic_edge_array *da, int dest);

void dynamic_edge_array_print(dynamic_edge_array da);

edge dynamic_edge_array_retrieve(dynamic_edge_array da, int index);

int dynamic_edge_array_resize(dynamic_edge_array *da, int size);

typedef struct dynamic_graph{
		dynamic_edge_array *edges;       //List of adjacency lists of vertices in array form
        int size;
        int maxn;
        } dynamic_graph;


int dynamic_graph_init(dynamic_graph *da, int initSize);

int dynamic_graph_resize(dynamic_graph *da, int size);

//Bi-directional link
int dynamic_graph_insert(dynamic_graph *da, int n1, int n2);

void dynamic_graph_print(dynamic_graph dg);

// Retrieve neighborhood dynamic array of given node
dynamic_edge_array dynamic_graph_retrieve_neighbors(dynamic_graph da, int node);

int dynamic_graph_reduce (dynamic_graph *dg);

int dynamic_graph_node_degree(dynamic_graph *dg, int index);

void dynamic_graph_free(dynamic_graph *dg);

void dynamic_edge_array_free(dynamic_edge_array *dea);

#endif
