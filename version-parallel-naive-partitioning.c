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
#include "version-parallel-naive-partitioning.h"

//#define DEBUG_TRACKING

// Global = Local + Offset
inline int naive_partitioning_convert_to_global_index(int local, int offset){
	return local + offset;
}

// Local = Global - Offset
inline int naive_partitioning_convert_to_local_index(int global, int offset){
	return global - offset;
}

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
	int base_partition_size;
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

	base_partition_size = input_dwg->size / number_of_partitions;

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

			free_partitions(*partitions, number_of_partitions);
			free(*partitions);

			return 0;
		} else {
			if(i != number_of_partitions-1)
				dynamic_weighted_graph_init(*(*partitions + i), base_partition_size);
			else
				dynamic_weighted_graph_init(*(*partitions + i), last_partition_size);
		}

	for(k = 0; k<number_of_partitions; k++) {
		local_offset = k*base_partition_size;
		for(i=k*base_partition_size;i<min((k+1)*base_partition_size, input_dwg->size);i++) {
			local_index = naive_partitioning_convert_to_local_index(i, local_offset);
			neighbors = input_dwg->edges+i;

			((*(*partitions + k))->edges + local_index)->self_loop = neighbors->self_loop;

			for(j=0;j<neighbors->count;j++) {
				dest = naive_partitioning_convert_to_local_index((neighbors->addr+j)->dest, local_offset);
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

// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
int naive_partitioning_get_neighbor_communities_list_weighted(sorted_linked_list *sll, dynamic_weighted_graph *partition_graph, int local_offset, community_developer *cd, int local_node_index, int *current_community_k_i_in) {
	dynamic_weighted_edge_array *neighbor_nodes;
	int i;

	int global_dest_node, global_dest_community, weight;

	int current_community;
	*current_community_k_i_in = 0;

	if(!partition_graph || !sll) {
		printf("Invalid input in computation of neighbor communities!");
		return 0;
	}

	if(local_node_index < 0 || local_node_index >= partition_graph->size) {
		printf("Invalid input in computation of neighbor communities: node out of range!");
		return 0;
	}

	current_community = *(cd->vertex_community + naive_partitioning_convert_to_global_index(local_node_index, local_offset));

	sorted_linked_list_free(sll);

	neighbor_nodes = partition_graph->edges + local_node_index;

	for(i = 0; i < neighbor_nodes->count; i++) {

		global_dest_node =  naive_partitioning_convert_to_global_index((neighbor_nodes->addr + i)->dest, local_offset);
		global_dest_community = *(cd->vertex_community + global_dest_node);
		weight = (neighbor_nodes->addr + i)->weight;

		if(global_dest_community == current_community)
			// Link internal to current node community
			*current_community_k_i_in += weight;
		else if(!(sorted_linked_list_insert(sll, global_dest_community, weight))) {
			printf("Cannot insert node in sorted linked list!");

			sorted_linked_list_free(sll);
			free(sll);

			return 0;
		}
	}

	return 1;
}

int phase_parallel_naive_partitioning_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
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

	int local_offset, global_index;

	int computation_failed;

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

	if(!dwg || minimum_improvement < MINIMUM_LEGAL_IMPROVEMENT || minimum_improvement > MAXIMUM_LEGAL_IMPROVEMENT) {
		printf("Invalid phase parameters!");

		return 0;
	}

	community_developer_init_weighted(&cd, dwg);

	community_developer_print(&cd,0);

	initial_phase_modularity = compute_modularity_weighted_reference_implementation_method_parallel(&cd);
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


	*(partitions_end) = (*(dwg_partitions))->size;
	for(partition_index = 1; partition_index < number_of_partitions; partition_index++)
		*(partitions_end+partition_index) = *(partitions_end+partition_index-1) + (*(dwg_partitions+partition_index))->size;

	computation_failed = 0;

#pragma omp parallel for schedule(dynamic,1) default(shared) private(local_offset, partition_graph,partition_start, partition_end, \
		final_iteration_modularity, initial_iteration_modularity, i, global_index, neighbors, partition_index, current_community, maximum_found_gain, \
	best_neighbor_community, current_community_k_i_in, removal_loss, neighbor, mcp, to_neighbor_modularity_delta, gain, \
	best_neighbor_community_k_i_in, exchange)
	for(partition_index = 0; partition_index < number_of_partitions; partition_index++)
	{
		partition_graph = *(dwg_partitions+partition_index);

		if(partition_index == 0)
			partition_start = 0;
		else
			partition_start = *(partitions_end + partition_index - 1);
		partition_end = *(partitions_end + partition_index);

		local_offset = partition_start;

#ifdef DEBUG_TRACKING
		printf("Thread %d analyzing partition %d from %d to %d", omp_get_thread_num(), partition_index, partition_start, partition_end);
#endif

		final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_range(&cd, partition_start, partition_end - 1);

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
			for(i = 0; i < partition_graph->size; i++) {
				global_index = naive_partitioning_convert_to_global_index(i, local_offset);

				// Node i, current edges: partition_graph->edges + i

				sorted_linked_list_init(&neighbors);

				current_community = *(cd.vertex_community + global_index);

				maximum_found_gain = 0;

				// By default, the best community corresponds to staying in the current one
				best_neighbor_community = current_community;

				// Compute neighbor communities and k_i_in
				// Please note that we still need to get the neighbor communities by looking for them by global indexes and not local
				// This modified function does the job
				if(!naive_partitioning_get_neighbor_communities_list_weighted(&neighbors,partition_graph, local_offset,&cd,i,&current_community_k_i_in)) {
					printf("Could not compute neighbor communities!\n");

#pragma omp atomic
					computation_failed++;
				} else {
					removal_loss = removal_modularity_loss_weighted(dwg, &cd, global_index, current_community_k_i_in);

					neighbor = neighbors.head;
					while(neighbor){
						// Note that by the definition of get_neighbor_communities_list_weighted current community isn't in the list

						// Compute modularity variation that would be achieved my moving node i from its current community to the neighbor community considered (neighbor->community)
						get_modularity_computing_package(&mcp,&cd,global_index,neighbor->community,neighbor->k_i_in);
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
						exchange.node = global_index;

						apply_transfer_weighted(dwg, &cd, &exchange);
					}
				}
			}

//			if(settings->verbose) {
//				printf("Iteration over all nodes complete.\n\n");
//			}

			final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_range(&cd, partition_start, partition_end - 1);

//			if(settings->verbose) {
//				printf("End of Iteration #%d.\n", phase_iteration_counter);
//				printf("Final iteration modularity: %f. Modularity gain: %f", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);
//			}
//
//			phase_iteration_counter++;

		} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);
	}

	if(computation_failed) {
		briefing->execution_successful = 0;

		free(partitions_end);
		free_partitions(dwg_partitions, number_of_partitions);
		free(dwg_partitions);
		community_developer_free(&cd);

		return 0;
	}

	free(partitions_end);
	free_partitions(dwg_partitions, number_of_partitions);
	free(dwg_partitions);

	final_phase_modularity = compute_modularity_weighted_reference_implementation_method_parallel(&cd);

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

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
