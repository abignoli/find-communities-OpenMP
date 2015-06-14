#include "community-computation-weighted.h"
#include "community-development.h"
#include "sorted-linked-list.h"
#include "dynamic-weighted-graph.h"
#include "silent-switch.h"
#include "execution-settings.h"
#include "community-exchange.h"
#include "community-computation-commons.h"
#include "execution-briefing.h"
#include "utilities.h"
#include <time.h>
#include <omp.h>
#include <stdlib.h>

#define PARTITIONS_PER_THREAD_MULTIPLIER 1
#define MINIMUM_PARTITION_SIZE 500

void free_partitions(dynamic_weighted_graph **partitions,int number_of_partitions) {
	int i;

	for(i=0;i<number_of_partitions;i++) {
		if(*(partitions+i))
			dynamic_weighted_graph_free(*(partitions+i));
		free(*(partitions+i));
	}
}

int generate_equal_node_partitions(dynamic_weighted_graph *input_dwg,int number_of_partitions, dynamic_weighted_graph ***partitions) {
	int i,j,k;
	int local_index;
	int local_offset;
	int dest, weight;
	int last_partition_size;
	int base_partition_size = input_dwg->size / number_of_partitions;
	int maximum_partitions_allowed;

	dynamic_weighted_edge_array *neighbors;

	if(number_of_partitions <= 0)  {
		printf("Partitions number must be greater than zero!\n");

		return 0;
	}

	maximum_partitions_allowed = input_dwg->size / MINIMUM_PARTITION_SIZE;

	if(input_dwg->size > maximum_partitions_allowed * MINIMUM_PARTITION_SIZE)
		maximum_partitions_allowed++;

	if(number_of_partitions > maximum_partitions_allowed)  {
		printf("Partitions number (%d) must be smaller than the maximum (%d) allowed by the minimum partition size (%d)!\n", number_of_partitions, maximum_partitions_allowed, MINIMUM_PARTITION_SIZE);

		return 0;
	}

	if(base_partition_size * number_of_partitions < input_dwg->size)
		base_partition_size++;

	last_partition_size = input_dwg->size - base_partition_size * (number_of_partitions - 1);

	if(!(*partitions = (dynamic_weighted_graph **) malloc(number_of_partitions * sizeof(dynamic_weighted_graph *)))) {
		printf("Couldn't allocate partition pointers array!\n");

		return 0;
	}

	for(i = 0; i < number_of_partitions; i++)
		*(*partitions + i) = NULL;

	for(i = 0; i < number_of_partitions; i++)
		if(!(*(*partitions + i) = (dynamic_weighted_graph *) malloc(number_of_partitions * sizeof(dynamic_weighted_graph)))) {
			printf("Couldn't allocate partitions!\n");

			if(i != number_of_partitions-1)
				dynamic_weighted_graph_init(*(*partitions + i), base_partition_size);
			else
				dynamic_weighted_graph_init(*(*partitions + i), last_partition_size);

			free_partitions(*partitions, number_of_partitions);
			free(*partitions);

			return 0;
		}

	for(k = 0; k<number_of_partitions; k++) {
		for(i=k*base_partition_size;i<min((k+1)*base_partition_size, input_dwg->size);i++) {
			local_offset = k*base_partition_size;
			local_index = i - local_offset;
			neighbors = input_dwg->edges+i;

			((*(*partitions + k))->edges)->self_loop = neighbors->self_loop;

			for(j=0;j<neighbors->count;j++) {
				dest = (neighbors->addr+j)->dest - local_offset;
				weight = (neighbors->addr+j)->weight;

				if(dest >= 0 && dest < base_partition_size) {
					// Then the destination belongs to the same partition, add edge
					if(!dynamic_weighted_graph_insert_force_directed(*(*partitions + k), local_index, dest, weight)) {
						printf("Could not insert edge in partition graph!\n");

						free_partitions(*partitions, number_of_partitions);
						free(*partitions);

						return 0;
					}
				}
			}
		}
	}

	return 1;
}

int phase_parallel_split_basic_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
	community_developer cd;

	double initial_phase_modularity, final_phase_modularity;
	double initial_iteration_modularity, final_iteration_modularity;

	sorted_linked_list neighbors;
	sorted_linked_list_elem *neighbor;
	modularity_computing_package mcp;
	dynamic_weighted_graph **dwg_partitions;
	dynamic_weighted_graph *partition_graph;
	community_exchange exchange;
	double to_neighbor_modularity_delta;
	int current_community_k_i_in;

	int maximum_partitions_allowed;

	double removal_loss;
	double gain;
	// Number of neighbor communities of a node worth considering for potential node transfer

	int number_of_partitions;
	int partition_index;

	double minimum_improvement = settings->minimum_iteration_improvement;

	int i;

//	int phase_iteration_counter = 0;
	int current_community, best_neighbor_community;
	int best_neighbor_community_k_i_in;

	int partition_start;
	int partition_end;

	int *partitions_end;

	double maximum_found_gain;

	int computation_failed;

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

	maximum_partitions_allowed = dwg->size / MINIMUM_PARTITION_SIZE;

	if(dwg->size > maximum_partitions_allowed * MINIMUM_PARTITION_SIZE)
		maximum_partitions_allowed++;

	number_of_partitions = omp_get_max_threads() * PARTITIONS_PER_THREAD_MULTIPLIER;

	if(number_of_partitions > maximum_partitions_allowed)
		number_of_partitions = maximum_partitions_allowed;

	if(!generate_equal_node_partitions(dwg, number_of_partitions ,&dwg_partitions)) {
		printf("Could not partition input graph!\n");
		briefing->execution_successful = 0;

		return 0;
	}

	if(!(partitions_end = (int *) malloc(number_of_partitions * sizeof(int)))) {
		printf("Could not create partitions end array!\n");
		briefing->execution_successful = 0;

		return 0;
	}

	// Just for clarity
	*(partitions_end+0) = 0;
	for(partition_index = 1; partition_index < number_of_partitions; partition_index++)
		*(partitions_end+partition_index) = *(partitions_end+partition_index-1);

	computation_failed = 0;

#pragma omp parallel for schedule(dynamic,1) default(shared) private(initial_iteration_modularity, final_iteration_modularity, i, partition_graph, partition_index, partition_start, partition_end, neighbors, current_community, maximum_found_gain, best_neighbor_community, current_community_k_i_in, removal_loss, neighbor, mcp, to_neighbor_modularity_delta, gain, best_neighbor_community_k_i_in, exchange) reduction(||:computation_failed)
	for(partition_index = 0; partition_index < number_of_partitions; partition_index++)
	{
		partition_graph = *(dwg_partitions+partition_index);

		if(partition_index == 0)
			partition_start = 0;
		else
			partition_start = *(partitions_end + partition_index - 1);
		partition_end = *(partitions_end + partition_index);

		computation_failed = 0;

		final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_range(&cd, partition_start, partition_end);

		do {
			initial_iteration_modularity = final_iteration_modularity;

//			if(settings->verbose) {
//				printf(PRINTING_UTILITY_SPARSE_DASHES);
//
//				printf(PRINTING_UTILITY_INDENT_TITLE);
//				printf("Iteration #%d\n\n", phase_iteration_counter);
//
//				printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);
//				printf("Starting iteration over all nodes...\n");
//			}

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

					computation_failed = 1;
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

//			if(settings->verbose) {
//				printf("Iteration over all nodes complete.\n\n");
//			}

			final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_range(&cd, partition_start, partition_end);

//			if(settings->verbose) {
//				printf("End of Iteration #%d.\n", phase_iteration_counter);
//				printf("Final iteration modularity: %f. Modularity gain: %f", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);
//			}
//
//			phase_iteration_counter++;

		} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);
	}

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

//	if(settings->verbose) {
//		printf(PRINTING_UTILITY_SPARSE_DASHES);
//		printf("End of phase\n\n");
//		printf("Number of iterations: %d\n", phase_iteration_counter);
//		printf("Final modularity: %f. Modularity gain: %f\n", final_phase_modularity, final_phase_modularity - initial_phase_modularity);
//	}

	briefing->execution_successful = 1;
	briefing->number_of_iterations = ILLEGAL_ITERATIONS_NUMBER;
	briefing->output_modularity = final_phase_modularity;

	return 1;
}
