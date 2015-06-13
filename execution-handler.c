#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "execution-settings.h"
#include "utilities.h"
#include "community-computation-weighted-sequential.h"
#include "community-computation-weighted.h"
#include <stdio.h>
#include <omp.h>

double execute_community_detection(dynamic_graph *input_dg, dynamic_weighted_graph *input_dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector) {
	if(!settings->sequential)
		omp_set_num_threads(settings->number_of_threads);

	if(settings->graph_type == NOT_WEIGHTED) {
		printf(PRINTING_NOT_YET_IMPLEMENTED);

		return 0;
	} else {
		if(settings->sequential)
			return sequential_find_communities_weighted(input_dwg, settings,community_graph, community_vector);
		else
			return parallel_find_communities_weighted(input_dwg, settings,community_graph, community_vector);
	}
}
