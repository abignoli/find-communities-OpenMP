#include "community-computation-weighted-sequential.h"
#include "community-computation-weighted.h"
#include "community-development.h"
#include "sorted-linked-list.h"
#include "community-exchange.h"
#include "dynamic-weighted-graph.h"
#include "silent-switch.h"

#ifdef SILENT_SWITCH_ON
#define printf(...)
#endif

double phase_weighted_sequential(dynamic_weighted_graph *dwg, double minimum_improvement, dynamic_weighted_graph **community_graph, int **community_vector) {
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

	int i;

	int phase_iteration_counter = 0;
	int current_community, best_neighbor_community;
	int best_neighbor_community_k_i_in;

	double maximum_found_gain;

	community_exchange exchange;

	if(!dwg || minimum_improvement < MINIMUM_LEGAL_IMPROVEMENT || minimum_improvement > MAXIMUM_LEGAL_IMPROVEMENT) {
		printf("Invalid phase parameters!");

		return ILLEGAL_MODULARITY_VALUE;
	}


	community_developer_init_weighted(&cd, dwg);

	community_developer_print(&cd,0);

	initial_phase_modularity = compute_modularity_weighted(dwg, &cd);
	final_iteration_modularity = initial_phase_modularity;

	printf("\n\n---------------- Phase start -----------------\n\n");
	printf("Initial phase modularity: %f\n", initial_phase_modularity);

	do {
		printf("\n\n---------------- Iteration #%d -----------------\n\n", phase_iteration_counter);

		initial_iteration_modularity = final_iteration_modularity;

		printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);

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

				return ILLEGAL_MODULARITY_VALUE;
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

		final_iteration_modularity = compute_modularity_weighted(dwg, &cd);

		printf("\n\nEnd of Iteration #%d - Result:\n\n", phase_iteration_counter);
		community_developer_print(&cd,0);

		phase_iteration_counter++;

		printf("Final iteration modularity: %f. Modularity gain: %f\n", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);

	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

	printf("Final phase modularity: %f. Modularity gain: %f\n", final_phase_modularity, final_phase_modularity - initial_phase_modularity);

	return final_phase_modularity - initial_phase_modularity;
}
