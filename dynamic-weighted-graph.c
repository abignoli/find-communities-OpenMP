#include <stdio.h>
#include <stdlib.h>
#include "dynamic-weighted-graph.h"
#include "shared-graph.h"
#include "silent-switch.h"

#ifdef SILENT_SWITCH_DYNAMIC_GRAPH_ON
#define printf(...)
#endif

int dynamic_weighted_edge_array_init(dynamic_weighted_edge_array *da, int initSize)
{
	da->size = initSize;
	da->count = 0;

	da->self_loop = 0;

	if( da->addr = (weighted_edge*) malloc(initSize * sizeof(weighted_edge)) )
		return 1;
	return 0;
}

int dynamic_weighted_edge_array_insert(dynamic_weighted_edge_array *da, int dest, int weight)
{
	int *tmp,*addr;
	int i;
	int new_size;

	// If more space is needed
	if(da->count >= da->size)
	{
		new_size = increase_size_policy(da->size, da->size+1);

		if(!dynamic_weighted_edge_array_resize(da, new_size))
		{
			printf("Run out of memory: can't insert data");

			return 0;
		}
	}

	(da->addr + da->count)->dest = dest;
	(da->addr + da->count)->weight = weight;
	da->count++;

	return 1;
}

void dynamic_weighted_edge_array_print(dynamic_weighted_edge_array da)
{
	int i;

	if(da.self_loop)
		printf("(Self %d) ", da.self_loop);

	for(i=0;i<da.count;i++)
		printf("(%d %d) ",(da.addr + i)->dest,(da.addr + i)->weight);

	return;
}

weighted_edge dynamic_weighted_edge_array_retrieve(dynamic_weighted_edge_array da, int index)
{
	return *(da.addr+index);
}

int dynamic_weighted_edge_array_resize(dynamic_weighted_edge_array *da, int size) {
	weighted_edge *tmp;
	int i;
	int to_copy;

	if(size == 0) {
		free(da->addr);
		da->addr=NULL;
	} else if(da->size != size) {
		if(tmp = (weighted_edge *) malloc((size) * sizeof(weighted_edge)) )
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

int dynamic_weighted_graph_init(dynamic_weighted_graph *da, int initSize)
{
	int i;

     da->size = initSize;
     da->maxn = -1;

     if( da->edges = (dynamic_weighted_edge_array*) malloc(initSize * sizeof(dynamic_weighted_edge_array)) ) {
    	 for(i = 0; i < initSize; i++)
    		 dynamic_weighted_edge_array_init(da->edges+i, DEFAULT_INIT_NEIGHBOURS_SIZE);

         return 1;
     }

    return 0;
}

int dynamic_weighted_graph_resize(dynamic_weighted_graph *da, int size) {
	dynamic_weighted_edge_array *new_edges;
	int to_copy,i;

	if(size != da->size) {
		// Change size
		if(new_edges = (dynamic_weighted_edge_array*) malloc(size * sizeof(dynamic_weighted_edge_array))) {
			to_copy = min(size, da->size);

			for(i=0; i<to_copy;i++) {
				*(new_edges+i) = *(da->edges+i);
			}

			free(da->edges);

			da->edges = new_edges;
			da->size = size;

			// da->maxn ???


			for(i=to_copy; i<size;i++) {
				dynamic_weighted_edge_array_init(da->edges+i, DEFAULT_INIT_NEIGHBOURS_SIZE);
			}
		} else {
	    	printf("Out of memory!\n");
	    	return 0;
		}

	}

	return 1;
}

//Bi-directional link
int dynamic_weighted_graph_insert(dynamic_weighted_graph *da, int n1, int n2, int weight)
{
    int *tmp,*addr;
    int i;

    int maxn;
    int new_size;

    // Check whether node 1 or 2 is not included in the graph yet
    maxn = max(n1,n2);

    if(maxn >= da->size) {
    	new_size = increase_size_policy(da->size, maxn+1);

    	if(!dynamic_weighted_graph_resize(da, new_size))
    		return 0;
    }

    if (n1 == n2)
    	(da->edges+n1)->self_loop = weight;
    else if(dynamic_weighted_edge_array_insert(da->edges+n1, n2, weight)) {
        if(dynamic_weighted_edge_array_insert(da->edges+n2, n1, weight)) {
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

//Directional link from n1 to n2
int dynamic_weighted_graph_insert_force_directed(dynamic_weighted_graph *da, int n1, int n2, int weight)
{
    int *tmp,*addr;
    int i;

    int maxn;
    int new_size;

    // Check whether node 1 or 2 is not included in the graph yet
    maxn = max(n1,n2);

    if(maxn >= da->size) {
    	new_size = increase_size_policy(da->size, maxn+1);

    	if(!dynamic_weighted_graph_resize(da, new_size))
    		return 0;
    }

    if (n1 == n2)
    	(da->edges+n1)->self_loop = weight;
    else if(dynamic_weighted_edge_array_insert(da->edges+n1, n2, weight)) {
    	da->maxn = max(da->maxn, maxn);
    } else {
    	printf("Out of memory!\n");
    	return 0;
    }

    return 1;
}

void dynamic_weighted_graph_print(dynamic_weighted_graph dg)
{
     int i;

     printf("\n\n------------ Printing graph ------------\n\n");

     printf("Number of nodes: %d\nMaximum observed vertex: %d\nEdges:\n", dg.size, dg.maxn);

     for(i=0;i<dg.size;i++) {
         printf("Node %d: ",i);
         dynamic_weighted_edge_array_print(*(dg.edges+i));
         printf("\n");
     }

     printf("\n\n------------ End of printing graph ------------\n\n");

     return;
}

// Retrieve neighborhood dynamic array of given node
dynamic_weighted_edge_array DynGraphRetrieveNeighbors(dynamic_weighted_graph da, int node)
{
    return *(da.edges+node);
}

int dynamic_weighted_graph_reduce (dynamic_weighted_graph *dg) {
	int i;

	dynamic_weighted_graph_resize(dg, dg->maxn+1);

	for(i = 0; i <= dg->maxn; i++)
		if(!dynamic_weighted_edge_array_resize((dg->edges + i), (dg->edges + i)->count)) {
			printf("Couldn't reduce dynamic array!\n");

			return 0;
		}

	return 1;
}

// Includes selfloop
int dynamic_weighted_graph_node_degree(dynamic_weighted_graph *dwg, int index) {
	int result, i;

	if(index < 0 || index > dwg->size)
		return -1;
	else {
		result = (dwg->edges+index)->self_loop;

		for(i = 0; i < (dwg->edges+index)->count; i++)
			result += ((dwg->edges+index)->addr+i)->weight;
	}

	return result;
}

int dynamic_weighted_graph_double_m(dynamic_weighted_graph *dwg) {
	int result = 0;
	int i;

	if(!dwg) {
		printf("dynamic_weighted_graph_double_m - Input graph is NULL!\n");

		return -1;
	}

	for(i = 0; i < dwg->size; i++)
		result += dynamic_weighted_graph_node_degree(dwg, i);

	return result;
}

void dynamic_weighted_graph_free(dynamic_weighted_graph *dwg) {
	int i;

	if(dwg->edges){
		for(i = 0; i < dwg->size; i++)
			dynamic_weighted_edge_array_free(dwg->edges + i);

		free(dwg->edges);
	}

	dwg->maxn = dwg->size = 0;
}

void dynamic_weighted_edge_array_free(dynamic_weighted_edge_array *dwea) {
	free(dwea->addr);
	dwea->count = dwea->self_loop = dwea->size = 0;
}

int dynamic_weighted_graph_self_loop(dynamic_weighted_graph *dwg, int node_index) {
	return (dwg->edges + node_index)->self_loop;
}
