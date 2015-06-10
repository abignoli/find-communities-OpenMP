#include "community-computation-weighted-sequential.h"
#include "community-computation-weighted.h"
#include "community-development.h"
#include "sorted-linked-list.h"
#include "community-exchange.h"
#include "dynamic-weighted-graph.h"
#include "silent-switch.h"
#include <time.h>

#include <stdio.h>

#ifdef SILENT_SWITCH_ON
#define printf(...)
#endif

double sequential_phase_weighted(dynamic_weighted_graph *dwg, double minimum_improvement, dynamic_weighted_graph **community_graph, int **community_vector) {
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

	initial_phase_modularity = compute_modularity_weighted_reference_implementation_method(&cd);
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

		final_iteration_modularity = compute_modularity_weighted_reference_implementation_method(&cd);

		printf("\n\nEnd of Iteration #%d - Result:\n\n", phase_iteration_counter);
		community_developer_print(&cd,0);

		phase_iteration_counter++;

		printf("Final iteration modularity: %f. Modularity gain: %f\n", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);

	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

	printf("Final phase modularity: %f. Modularity gain: %f\n", final_phase_modularity, final_phase_modularity - initial_phase_modularity);

	return final_phase_modularity;
}

double sequential_find_communities_weighted(dynamic_weighted_graph *dwg, double minimum_phase_improvement, double minimum_iteration_improvement, char *output_communities_filename, char *output_graphs_filename, dynamic_weighted_graph **community_graph, int **community_vector) {
	int phase_counter;

	dynamic_weighted_graph *phase_output_community_graph;
	int *phase_output_community_vector;

	double initial_phase_modularity, final_phase_modularity;

	FILE *output_communities_file;
	FILE *output_graphs_file;



	if(!dwg || !valid_minimum_improvement(minimum_phase_improvement) || !valid_minimum_improvement(minimum_iteration_improvement)) {
		printf("Invalid algorithm parameters!");

		return ILLEGAL_MODULARITY_VALUE;
	}

	if(output_communities_filename) {
		output_communities_file = fopen (output_communities_filename, "w+");

		if(!output_communities_file) {
			printf("Could not open output communities file: %s", output_communities_filename);

			return ILLEGAL_MODULARITY_VALUE;
		}
	}

	if(output_graphs_filename) {
		output_graphs_file = fopen (output_graphs_filename, "w+");

		if(!output_graphs_file) {
			printf("Could not open output graphs file: %s", output_graphs_filename);

			fclose(output_communities_file);
			return ILLEGAL_MODULARITY_VALUE;
		}
	}

	*community_vector = NULL;
	phase_output_community_graph = NULL;

	phase_counter = 0;
	final_phase_modularity = compute_modularity_init_weighted_reference_implementation_method(dwg);


	do {
		free(*community_vector);

		initial_phase_modularity = final_phase_modularity;

		printf("\n\nPHASE #%d:\n\nGraph size: %d\nInitial phase modularity: %f (Re-computed: %f)\n",phase_counter, dwg->size, final_phase_modularity, compute_modularity_init_weighted_reference_implementation_method(dwg));

		dynamic_weighted_graph_print(*dwg);

		if(sequential_phase_weighted(dwg,minimum_iteration_improvement,&phase_output_community_graph, community_vector) == ILLEGAL_MODULARITY_VALUE) {
			printf("Bad phase #%d computation!\n",phase_counter);

			return ILLEGAL_MODULARITY_VALUE;
		}

		if(output_communities_file && !output_save_communities(output_communities_file, *community_vector, dwg->size)) {
			printf("Couldn't save communities output of phase #%d!\n",phase_counter);

			return ILLEGAL_MODULARITY_VALUE;
		}
		if(output_communities_file && !output_save_community_graph(output_graphs_file, phase_output_community_graph, phase_counter)) {
			printf("Couldn't save graph output of phase #%d!\n",phase_counter);

			return ILLEGAL_MODULARITY_VALUE;
		}

		final_phase_modularity = compute_modularity_community_vector_weighted(dwg,*community_vector);

		printf("-- End of Phase #%d - Initial modularity: %f - Final modularity: %f - Gain: %f\n", phase_counter, initial_phase_modularity, final_phase_modularity, final_phase_modularity - initial_phase_modularity);

		// Clean memory
		// Avoids freeing initial input graph
		if(phase_counter > 0)
			dynamic_weighted_graph_free(dwg);

		// Prepare for next phase
		phase_counter++;
		dwg = phase_output_community_graph;
	} while(final_phase_modularity - initial_phase_modularity > minimum_phase_improvement);

	*community_graph = phase_output_community_graph;

	if(output_communities_file)
		fclose(output_communities_file);

	if(output_graphs_file)
		fclose(output_graphs_file);

	return final_phase_modularity;
}

