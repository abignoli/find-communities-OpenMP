/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "community-computation-weighted-sequential.h"
#include "community-computation-weighted.h"
#include "community-development.h"
#include "sorted-linked-list.h"
#include "community-exchange.h"
#include "dynamic-weighted-graph.h"
#include "silent-switch.h"
#include "execution-settings.h"
#include "community-computation-commons.h"
#include "execution-briefing.h"
#include "utilities.h"
#include <time.h>
#include "printing_controller.h"
#include "vertex-following.h"
#include <stdio.h>
#include <stdlib.h>


// Designed to be called just before the first phase, not after
int pre_compute_vertex_following(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
	community_developer cd;

	dynamic_weighted_edge_array *neighbors;

	int i;

	int reduced_nodes;

	community_exchange exchange;

	if(!dwg) {
		printf("Invalid pre_compute_vertex_following parameters!");

		return 0;
	}


	community_developer_init_weighted(&cd, dwg);


	if(settings->verbose) {
		printf(PRINTING_UTILITY_SPARSE_DASHES);

		printf("pre_compute_vertex_following - Starting iteration over all nodes...\n");
	}


	reduced_nodes = 0;

#pragma omp parallel for private(i, neighbors, exchange) reduction(+:reduced_nodes)
	for(i = 0; i < dwg->size; i++) {
		neighbors = dwg->edges + i;
		if(neighbors->count == 1 && (neighbors->addr)->dest > i) {
			// If node i has just one neighbor
			// and that neighbor has index (label) greater than i
			// join its community

			exchange.dest = (neighbors->addr)->dest;
			exchange.k_i_in_dest = (neighbors->addr)->weight;
			exchange.k_i_in_src = 0;
			exchange.node = i;

			apply_transfer_weighted(dwg, &cd, &exchange);

			reduced_nodes++;
		}
	}


	if(settings->verbose) {
		printf("Iteration over all nodes complete.\n");
		printf("Reduced %d nodes.\n\n", reduced_nodes);
	}

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	briefing->execution_successful = 1;

	return 1;
}
