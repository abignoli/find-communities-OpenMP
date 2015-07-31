/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>

#include "dynamic-weighted-graph.h"

int community_vector_init(int **community_vector, int size) {
	int i;

	if(!(*community_vector = (int*) malloc(size * sizeof(int))))
		return 0;

	for(i = 0; i < size; i++)
		**community_vector = i;

	return 1;
}

int output_save_communities(FILE *output_communities_file,int *community_vector, int size) {
	int i;

	if(!output_communities_file) {
		printf("output_save_communities - Can't save communities: file is NULL!\n");

		return 0;
	}

	for(i = 0; i < size; i++)
		fprintf(output_communities_file, "%d %d\n", i, *(community_vector+i));

	return 1;
}

int output_save_community_graph(FILE *output_graphs_file, dynamic_weighted_graph *output_graph, int phase_counter) {
	int i,j;
	dynamic_weighted_edge_array *neighbors;

	if(!output_graphs_file) {
		printf("output_save_community_graph - Can't save community graph: file is NULL!\n");

		return 0;
	}

	fprintf(output_graphs_file, "--------- OUTPUT GRAPH OF PHASE #%d ---------\n\n", phase_counter);

	for(i = 0; i < output_graph->size; i++) {
		neighbors = output_graph->edges + i;
		if(neighbors->self_loop)
			fprintf(output_graphs_file, "%d %d %d\n", i, i, neighbors->self_loop);

		for(j = 0; j < neighbors->count; j++)
			fprintf(output_graphs_file, "%d %d %d\n", i, (neighbors->addr +j)->dest, (neighbors->addr +j)->weight);
	}

	return 1;
}
