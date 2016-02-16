/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include "dynamic-graph.h"
#include "shared-graph.h"
#include "silent-switch.h"
#include "dynamic-weighted-graph.h"
#include "utilities.h"

//#ifdef SILENT_SWITCH_DYNAMIC_GRAPH_ON
//#define printf(...)
//#endif

int dynamic_edge_array_init(dynamic_edge_array *da, int initSize)
{
	da->size = initSize;
	da->count = 0;

	da->self_loop = 0;

	if( da->addr = (edge*) malloc(initSize * sizeof(edge)) )
		return 1;
	return 0;
}

int dynamic_edge_array_insert(dynamic_edge_array *da, int dest)
{
	int *tmp;
	int new_size;

	// If more space is needed
	if(da->count >= da->size)
	{
#define SILENT
		new_size = increase_size_policy(da->size, da->size+1);

		if(!dynamic_edge_array_resize(da, new_size))
		{
			printf("Run out of memory: can't insert data");

			return 0;
		}
	}

	(da->addr + da->count)->dest = dest;
	da->count++;

	return 1;
}

void dynamic_edge_array_print(dynamic_edge_array da)
{
	int i;

	if(da.self_loop)
		printf("Self ");

	for(i=0;i<da.count;i++)
		printf("%d ",(da.addr + i)->dest);

	return;
}

edge dynamic_edge_array_retrieve(dynamic_edge_array da, int index)
{
	return *(da.addr+index);
}

int dynamic_edge_array_resize(dynamic_edge_array *da, int size) {
	edge *tmp;
	int i;
	int to_copy;

	if(size == 0) {
		free(da->addr);
		da->addr=NULL;
	} else if(da->size != size) {
		if(tmp = (edge *) malloc((size) * sizeof(edge)) )
		{
			to_copy = min(da->count, size);

			for(i=0;i<to_copy;i++)
				*(tmp + i) = *(da->addr + i);
			free(da->addr); //Free previous array
			da->addr = tmp;

			da->size=size;
			da->count = to_copy;
		} else {
			printf("Run out of memory: can't insert data");

			return 0;
		}
	}

	return 1;
}

int dynamic_graph_init(dynamic_graph *da, int initSize)
{
	int i;

     da->size = initSize;
     da->maxn = -1;

     if( da->edges = (dynamic_edge_array*) malloc(initSize * sizeof(dynamic_edge_array)) ) {
    	 for(i = 0; i < initSize; i++)
    		 dynamic_edge_array_init(da->edges+i, DEFAULT_INIT_NEIGHBOURS_SIZE);

         return 1;
     }

    return 0;
}

int dynamic_graph_resize(dynamic_graph *da, int size) {
	dynamic_edge_array *new_edges;
	int to_copy,i;

	if(size != da->size) {
		// Change size
		if(new_edges = (dynamic_edge_array*) malloc(size * sizeof(dynamic_edge_array))) {
			to_copy = min(size, da->size);

			for(i=0; i<to_copy;i++) {
				*(new_edges+i) = *(da->edges+i);
			}

			free(da->edges);

			da->edges = new_edges;
			da->size = size;

			// da->maxn ???


			for(i=to_copy; i<size;i++) {
				dynamic_edge_array_init(da->edges+i, DEFAULT_INIT_NEIGHBOURS_SIZE);
			}
		} else {
	    	printf("Out of memory!\n");
	    	return 0;
		}

	}

	return 1;
}

//Bi-directional link
int dynamic_graph_insert(dynamic_graph *da, int n1, int n2)
{
    int *tmp;

    int maxn;
    int new_size;

    // Check whether node 1 or 2 is not included in the graph yet
    maxn = max(n1,n2);

    if(maxn >= da->size) {
    	new_size = increase_size_policy(da->size, maxn+1);

    	if(!dynamic_graph_resize(da, new_size))
    		return 0;
    }

    if (n1 == n2)
    	(da->edges+n1)->self_loop = 1;
    else if(dynamic_edge_array_insert(da->edges+n1, n2)) {
        if(dynamic_edge_array_insert(da->edges+n2, n1)) {
        	da->maxn = max(da->maxn, maxn);
        } else {
        	printf("Out of memory!\n");
        	return 0;
        }
    } else {
    	printf("Out of memory!\n");
    	return 0;
    }

    return 1;
}

void dynamic_graph_print(dynamic_graph dg)
{
     int i;

     printf("\n\n------------ Printing graph ------------\n\n");

     printf("Number of nodes: %d\nMaximum observed vertex: %d\nEdges:\n", dg.size, dg.maxn);

     for(i=0;i<dg.size;i++) {
         printf("Node %d: ",i);
         dynamic_edge_array_print(*(dg.edges+i));
         printf("\n");
     }

     printf("\n\n------------ End of printing graph ------------\n\n");

     return;
}

// Retrieve neighborhood dynamic array of given node
dynamic_edge_array dynamic_graph_retrieve_neighbors(dynamic_graph da, int node)
{
    return *(da.edges+node);
}

int dynamic_graph_reduce (dynamic_graph *dg) {
	int i;

	dynamic_graph_resize(dg, dg->maxn+1);

	for(i = 0; i <= dg->maxn; i++)
		if(!dynamic_edge_array_resize((dg->edges + i), (dg->edges + i)->count)) {
			printf("Couldn't reduce dynamic array!\n");

			return 0;
		}

	return 1;
}

// Includes selfloop
int dynamic_graph_node_degree(dynamic_graph *dg, int index) {
	int result;

	if(index < 0 || index > dg->size)
		return -1;
	else
		result = (dg->edges+index)->self_loop + (dg->edges+index)->count;

	return result;
}

// Given weight will be assigned to each node in the graph
int convert_to_weighted(dynamic_graph *dg, dynamic_weighted_graph *dwg, int weight){
	int i,j;

	dynamic_edge_array original_neighbors;

	if(!dg || !dwg) {
		printf("convert_to_weighted - null dg or dwg!\n");

		return 0;
	}

	if(!dynamic_weighted_graph_init(dwg,DEFAULT_INIT_NODES_SIZE))
		return 0;

	for(i = 0; i < dg->size; i++) {
		original_neighbors = *(dg->edges + i);
		for(j = 0; j < original_neighbors.count; j++)
			if(!dynamic_weighted_graph_insert(dwg,i,(original_neighbors.addr + j)->dest, weight)) {
				printf("Could not insert edge %d - %d!\n", i, (original_neighbors.addr + j)->dest);
				return 0;
			}
	}

	// Reduce to optimal size
	if(!dynamic_weighted_graph_reduce(dwg)) {
		printf("Could not reduce graph size!\n");

		return 0;
	}

	return 1;
}

void dynamic_graph_free(dynamic_graph *dg) {
	int i;

	if(dg->edges){
		for(i = 0; i < dg->size; i++)
			dynamic_edge_array_free(dg->edges + i);

		free(dg->edges);
	}

	dg->maxn = dg->size = 0;
}

void dynamic_edge_array_free(dynamic_edge_array *dea) {
	free(dea->addr);
	dea->count = dea->self_loop = dea->size = 0;
}
