/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef DYNAMIC_WEIGHTED_GRAPH_H
#define DYNAMIC_WEIGHTED_GRAPH_H

#define DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES 1

//DynamicGraph

typedef struct weighted_edge {
	int dest;
	int weight;
} weighted_edge;

typedef struct dynamic_weighted_edge_array{
	weighted_edge *addr;
	int self_loop;
	int count;
	int size;
} dynamic_weighted_edge_array;

int dynamic_weighted_edge_array_init(dynamic_weighted_edge_array *da, int initSize);

int dynamic_weighted_edge_array_insert(dynamic_weighted_edge_array *da, int dest, int weight);

void dynamic_weighted_edge_array_print(dynamic_weighted_edge_array da);

weighted_edge dynamic_weighted_edge_array_retrieve(dynamic_weighted_edge_array da, int index);

int dynamic_weighted_edge_array_resize(dynamic_weighted_edge_array *da, int size);

void dynamic_weighted_edge_array_free(dynamic_weighted_edge_array *dwea);

typedef struct dynamic_weighted_graph{
		dynamic_weighted_edge_array *edges;       //List of adjacency lists of vertices in array form
        int size;
        int maxn;
        } dynamic_weighted_graph;


int dynamic_weighted_graph_init(dynamic_weighted_graph *da, int initSize);

int dynamic_weighted_graph_resize(dynamic_weighted_graph *da, int size);

//Bi-directional link
int dynamic_weighted_graph_insert(dynamic_weighted_graph *da, int n1, int n2, int weight);

void dynamic_weighted_graph_print(dynamic_weighted_graph dg);

// Retrieve neighborhood dynamic array of given node
dynamic_weighted_edge_array DynGraphRetrieveNeighbors(dynamic_weighted_graph da, int node);

int dynamic_weighted_graph_reduce (dynamic_weighted_graph *dg);

int dynamic_weighted_graph_node_degree(dynamic_weighted_graph *dg, int index);

void dynamic_weighted_graph_free(dynamic_weighted_graph *dwg);

int dynamic_weighted_graph_self_loop(dynamic_weighted_graph *dwg, int node_index);

int dynamic_weighted_graph_double_m(dynamic_weighted_graph *dwg);

int dynamic_weighted_graph_insert_force_directed(dynamic_weighted_graph *da, int n1, int n2, int weight);

#endif
