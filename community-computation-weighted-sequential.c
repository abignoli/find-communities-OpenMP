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

#include <stdio.h>
#include <stdlib.h>

//#ifdef SILENT_SWITCH_ON
//#define printf(...)
//#endif

int sequential_phase_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
	community_developer cd;

	double initial_phase_modularity, final_phase_modularity;
	double initial_iteration_modularity, final_iteration_modularity;

	sorted_linked_list neighbors;
	sorted_linked_list_elem *neighbor;
	modularity_computing_package mcp;
	double to_neighbor_modularity_delta;
	int current_community_k_i_in;
	double removal_loss;
	double gain;
	// Number of neighbor communities of a node worth considering for potential node transfer

	double minimum_improvement = settings->minimum_iteration_improvement;

	int i;

	int phase_iteration_counter = 0;
	int current_community, best_neighbor_community;
	int best_neighbor_community_k_i_in;

	double maximum_found_gain;

	community_exchange exchange;

	if(!dwg || minimum_improvement < MINIMUM_LEGAL_IMPROVEMENT || minimum_improvement > MAXIMUM_LEGAL_IMPROVEMENT) {
		printf("Invalid phase parameters!");

		return 0;
	}


	community_developer_init_weighted(&cd, dwg);

	community_developer_print(&cd,0);

	initial_phase_modularity = compute_modularity_weighted_reference_implementation_method(&cd);
	final_iteration_modularity = initial_phase_modularity;

	if(settings->verbose) {
		printf(PRINTING_UTILITY_SPARSE_DASHES);

		printf(PRINTING_UTILITY_INDENT_TITLE);
		printf("Phase start\n\n");

		printf("Initial phase modularity: %f\n", initial_phase_modularity);
	}

	do {
		initial_iteration_modularity = final_iteration_modularity;

		if(settings->verbose) {
			printf(PRINTING_UTILITY_SPARSE_DASHES);

			printf(PRINTING_UTILITY_INDENT_TITLE);
			printf("Iteration #%d\n\n", phase_iteration_counter);

			printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);
			printf("Starting iteration over all nodes...\n");
		}

		// Do the sequential iterations over all nodes
		for(i = 0; i < dwg->size; i++) {

			// Node i, current edges: dwg->edges + i

			sorted_linked_list_init(&neighbors);

			current_community = *(cd.vertex_community + i);

			maximum_found_gain = 0;

			// By default, the best community corresponds to staying in the current one
			best_neighbor_community = current_community;

			// Compute neighbor communities and k_i_in
			if(!get_neighbor_communities_list_weighted(&neighbors,dwg,&cd,i,&current_community_k_i_in)) {
				printf("Could not compute neighbor communities!\n");

				return 0;
			}

			removal_loss = removal_modularity_loss_weighted(dwg, &cd, i, current_community_k_i_in);

			neighbor = neighbors.head;
			while(neighbor){
				// Note that by the definition of get_neighbor_communities_list_weighted current community isn't in the list

				// Compute modularity variation that would be achieved my moving node i from its current community to the neighbor community considered (neighbor->community)
				get_modularity_computing_package(&mcp,&cd,i,neighbor->community,neighbor->k_i_in);
				to_neighbor_modularity_delta = modularity_delta(&mcp);

				// Check if the gain is worth to be considered
				gain = to_neighbor_modularity_delta - removal_loss;
				if(gain > MINIMUM_TRANSFER_GAIN && gain > maximum_found_gain) {
					maximum_found_gain = gain;
					best_neighbor_community = neighbor->community;
					best_neighbor_community_k_i_in = neighbor->k_i_in;
				}

				neighbor = neighbor->next;
			}

			sorted_linked_list_free(&neighbors);

			if(best_neighbor_community != current_community) {
				// Apply transfer
				exchange.dest = best_neighbor_community;
				exchange.k_i_in_dest = best_neighbor_community_k_i_in;
				exchange.k_i_in_src = current_community_k_i_in;
				exchange.modularity_delta = maximum_found_gain;
				exchange.node = i;

				apply_transfer_weighted(dwg, &cd, &exchange);
			}
		}

		if(settings->verbose) {
			printf("Iteration over all nodes complete.\n\n");
		}

		final_iteration_modularity = compute_modularity_weighted_reference_implementation_method(&cd);

		if(settings->verbose) {
			printf("End of Iteration #%d.\n", phase_iteration_counter);
			printf("Final iteration modularity: %f. Modularity gain: %f", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);
		}

		phase_iteration_counter++;

	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

	if(settings->verbose) {
		printf(PRINTING_UTILITY_SPARSE_DASHES);
		printf("End of phase\n\n");
		printf("Number of iterations: %d\n", phase_iteration_counter);
		printf("Final modularity: %f. Modularity gain: %f\n", final_phase_modularity, final_phase_modularity - initial_phase_modularity);
	}

	briefing->execution_successful = 1;
	briefing->number_of_iterations = phase_iteration_counter;
	briefing->output_modularity = final_phase_modularity;

	return 1;
}

