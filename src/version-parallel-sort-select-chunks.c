/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "community-computation-weighted.h"
#include "temporary-community-edge.h"
#include "community-computation-commons.h"
#include "neighbor-computation-package.h"

#include "community-development.h"
#include "dynamic-weighted-graph.h"
#include "community-exchange.h"
#include "silent-switch.h"
#include "sorted-linked-list.h"
#include "utilities.h"
#include "version-parallel-sort-select-chunks.h"

#include "execution-settings.h"
#include "execution-briefing.h"

#include "printing_controller.h"

#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

int init_neighbor_computation_package_threads(neighbor_computation_package **ncp_threads, int number_of_threads, int number_of_communities) {
	int i;

	if(!(*ncp_threads = (neighbor_computation_package *) malloc (number_of_threads * sizeof(neighbor_computation_package)))) {
		printf("init_neighbor_computation_package_threads - Could not allocate ncp array!\n");
		return 0;
	}

	// No parallel, internal ncp init is already parallel
	for(i=0;i<number_of_threads;i++)
		if(!neighbor_computation_package_init(*ncp_threads + i, number_of_communities)) {
			printf("init_neighbor_computation_package_threads - Could not init ncps!\n");
			return 0;
		}

	return 1;
}

int get_neighbor_computation_package_weighted(dynamic_weighted_graph *dwg, community_developer *cd, int node_index, neighbor_computation_package *ncp, int *current_community_k_i_in) {
	dynamic_weighted_edge_array *neighbor_nodes;
	int i;

	int current_community;
	int dest_community;
	int weight;
	*current_community_k_i_in = 0;

	if(!dwg || !ncp) {
		printf("Invalid input in computation of neighbor communities!");
		return 0;
	}

	if(node_index < 0 || node_index >= dwg->size) {
		printf("Invalid input in computation of neighbor communities: node out of range!");
		return 0;
	}

	current_community = *(cd->vertex_community + node_index);

	neighbor_nodes = dwg->edges + node_index;

	for(i = 0; i < neighbor_nodes->count; i++) {
		dest_community = *(cd->vertex_community + (neighbor_nodes->addr + i)->dest);
		weight = (neighbor_nodes->addr + i)->weight;

		if(dest_community == current_community)
			// Link internal to current node community
			*current_community_k_i_in += weight;
		else if(!(neighbor_computation_package_insert(ncp, dest_community, weight))) {
			printf("Cannot insert community in ncp!\n");
			return 0;
		}
	}

	return 1;
}

int phase_parallel_sort_select_chunks_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
	community_developer cd;

	double initial_phase_modularity, final_phase_modularity;
	double initial_iteration_modularity, final_iteration_modularity;

	modularity_computing_package mcp;
	double to_neighbor_modularity_delta;
	int current_community_k_i_in;
	double removal_loss;
	double gain;
	// Number of neighbor communities of a node worth considering for potential node transfer
	int number_of_neighbor_communities;

	double minimum_improvement = settings->minimum_iteration_improvement;

	community_exchange *node_exchanges_base_pointer;

	int base,total_exchanges;
	int exchange_index;

	community_exchange *sorted_output_multi_thread;
	// True if it is actually used (multi-threading)
	int sorted_output_multi_thread_needs_free;

	short *selected;
	int stop_scanning_position;

	int i,j;

	int actual_number_of_threads;

	int phase_iteration_counter = 0;

	int neighbor_communities_bad_computation;

	// Time measurements
	double wtime_phase_begin, wtime_phase_init_cd_begin, wtime_phase_init_cd_end, wtime_phase_output_translation_begin, wtime_phase_output_translation_end, wtime_phase_end;
	double wtime_iteration_begin, wtime_iteration_node_scan_begin, wtime_iteration_node_scan_end, wtime_iteration_edge_compression_begin, wtime_iteration_edge_compression_end, wtime_iteration_edge_sorting_begin, wtime_iteration_edge_sorting_end, wtime_iteration_edge_selection_begin, wtime_iteration_edge_selection_end, wtime_iteration_edge_apply_begin, wtime_iteration_edge_apply_end, wtime_iteration_end;
	int full_iteration;
	double wtime_iteration, wtime_iteration_node_scan, wtime_iteration_edge_compression, wtime_iteration_edge_sorting, wtime_iteration_edge_selection, wtime_iteration_edge_apply;
	double wtime_iteration_duration_avg;

	int chunk_size = DEFAULT_SORT_SELECT_CHUNKS_CHUNK_SIZE;
	int number_of_chunks, chunk_index;
	int chunk_start, chunk_end, chunk_edge_start;

	neighbor_computation_package *ncp_threads;
	neighbor_computation_package *ncp_thread;
	int neighbor_community, neighbor_community_k_i_in;

	wtime_phase_begin = omp_get_wtime();

	// Minimum improvement refers to iteration improvement
	if(!dwg || !valid_minimum_improvement(minimum_improvement)) {
		printf("Invalid phase parameters!\n");
		briefing->execution_successful = 0;
		return 0;
	}

	if(settings->execution_settings_sort_select_chunks_chunk_size > 0)
		chunk_size = settings->execution_settings_sort_select_chunks_chunk_size;

	number_of_chunks = dwg->size / chunk_size;
	if(number_of_chunks * chunk_size < dwg->size)
		number_of_chunks++;

	wtime_phase_init_cd_begin = omp_get_wtime();

	community_developer_init_weighted(&cd, dwg);

	wtime_phase_init_cd_end = omp_get_wtime();

#pragma omp parallel
	{
#pragma omp single
		{
			actual_number_of_threads = omp_get_num_threads();
		}
	}

	if(!init_neighbor_computation_package_threads(&ncp_threads,actual_number_of_threads,dwg->size)) {
		printf("Could not init thread ncps!\n");
		briefing->execution_successful = 0;
		return 0;
	}

	initial_phase_modularity = compute_modularity_weighted_reference_implementation_method_parallel(&cd);
	final_iteration_modularity = initial_phase_modularity;

	if(settings->verbose) {
		printf(PRINTING_UTILITY_SPARSE_DASHES);

		printf(PRINTING_UTILITY_INDENT_TITLE);
		printf("Phase start\n\n");

		printf("Initial phase modularity: %f\n", initial_phase_modularity);
	}

#ifdef VERBOSE_TABLE
	if(settings->verbose) {
		printf(PRINTING_SORT_SELECT_TIME_TABLE_HEADER);
	}
#endif

	wtime_iteration_duration_avg = 0;

	do {

		full_iteration = 0;

		wtime_iteration_begin = omp_get_wtime();

		initial_iteration_modularity = final_iteration_modularity;

#ifndef VERBOSE_TABLE

			if(settings->verbose) {
				printf(PRINTING_UTILITY_SPARSE_DASHES);

				printf(PRINTING_UTILITY_INDENT_TITLE);
				printf("Iteration #%d\n\n", phase_iteration_counter);

				printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);
				printf("Starting iteration over all chunks...\n");
			}

#endif

		// Start chunk partitioned iterations


		for(chunk_index = 0; chunk_index < number_of_chunks; chunk_index++) {

			chunk_start = chunk_index * chunk_size;
			chunk_end = min((chunk_index + 1) * chunk_size, dwg->size);

			if(chunk_start == 0)
				chunk_edge_start = 0;
			else
				chunk_edge_start = *(cd.cumulative_edge_number + chunk_start - 1);

			neighbor_communities_bad_computation = 0;

			sorted_output_multi_thread_needs_free = 0;

			total_exchanges = 0;

			wtime_iteration_node_scan_begin = omp_get_wtime();

#pragma omp parallel default(shared) private(ncp_thread)
			{
				ncp_thread = ncp_threads + omp_get_thread_num();

				// Do the parallel iterations over all nodes
				// TODO put Parallel for - Everything else is good to go
#pragma omp for  schedule(dynamic,PARALLEL_SORT_SELECT_CHUNKS_NODE_ITERATION_CHUNK_SIZE) reduction(+:total_exchanges) private(i, node_exchanges_base_pointer, \
				number_of_neighbor_communities, current_community_k_i_in, removal_loss, mcp, to_neighbor_modularity_delta, gain, neighbor_community, neighbor_community_k_i_in)
				for(i = chunk_start; i < chunk_end; i++) {

					//			printf("NODE ITERATION - Thread #%d on node %d\n", omp_get_thread_num(), i);

					if(i > 0)
						node_exchanges_base_pointer = cd.exchange_ranking + *(cd.cumulative_edge_number + i - 1);
					else
						node_exchanges_base_pointer = cd.exchange_ranking;

					// Node i, current edges: dwg->edges + i

					number_of_neighbor_communities = 0;

					// Compute neighbor communities and k_i_in
					if(get_neighbor_computation_package_weighted(dwg,&cd,i,ncp_thread,&current_community_k_i_in)) {

						removal_loss = removal_modularity_loss_weighted(dwg, &cd, i, current_community_k_i_in);

						while(neighbor_computation_package_pop_and_clean(ncp_thread, &neighbor_community, &neighbor_community_k_i_in)){
							// Note that by the definition of get_neighbor_communities_list_weighted current community isn't in the list

							// Compute modularity variation that would be achieved my moving node i from its current community to the neighbor community considered (neighbor->community)
							get_modularity_computing_package(&mcp,&cd,i,neighbor_community,neighbor_community_k_i_in);
							to_neighbor_modularity_delta = modularity_delta(&mcp);

							// Check if the gain is worth to be considered
							gain = to_neighbor_modularity_delta - removal_loss;
							if(gain > MINIMUM_TRANSFER_GAIN) {
								// Put computed potential transfer in its correct position for later community pairing selection

								set_exchange_ranking(node_exchanges_base_pointer + number_of_neighbor_communities,i,neighbor_community,current_community_k_i_in, neighbor_community_k_i_in,gain);

								number_of_neighbor_communities += 1;

							}
						}

						*(cd.vertex_neighbor_communities + i) = number_of_neighbor_communities;
						total_exchanges+=number_of_neighbor_communities;
					} else
#pragma omp atomic
						neighbor_communities_bad_computation++;
				}
			}


			if(neighbor_communities_bad_computation) {
				printf("Could not compute neighbor communities!\n");
				briefing->execution_successful = 0;
				return 0;
			}


			wtime_iteration_node_scan_end = omp_get_wtime();

#ifndef VERBOSE_TABLE
			if(settings->verbose) {
				printf("Iteration over all nodes complete.\n\n");
				printf("Starting potential node transfers compression...\n");
			}
#endif

			if(total_exchanges > 0) {

				full_iteration = 1;

				// -------------------------------- SEQUENTIAL SECTION

				wtime_iteration_edge_compression_begin = omp_get_wtime();

				// Start copying in the first free position
				base = chunk_start;

				exchange_index = chunk_edge_start;

				do {
					exchange_index += *(cd.vertex_neighbor_communities+base);
					base++;
				}while(base < chunk_end && *(cd.cumulative_edge_number+base-1) == exchange_index);

				for(i = base; i < chunk_end; i++)
					for(j = 0; j < *(cd.vertex_neighbor_communities + i); j++) {
						*(cd.exchange_ranking + exchange_index) = *(cd.exchange_ranking + *(cd.cumulative_edge_number + i - 1) + j);

						exchange_index++;
					}

				// total_exchanges represents the total number of active exchanges to be sorted

				wtime_iteration_edge_compression_end = omp_get_wtime();

				// -------------------------------- END OF SEQUENTIAL SECTION

#ifndef VERBOSE_TABLE
				if(settings->verbose) {
					printf("Potential node transfers compression complete.\n\n");
					printf("Starting  potential node transfers sorting...\n");
				}
#endif

				// Do the parallel sorting of the community pairings array

				wtime_iteration_edge_sorting_begin = omp_get_wtime();

				if(!community_exchange_parallel_quick_sort_main(cd.exchange_ranking + chunk_edge_start, total_exchanges, settings, &sorted_output_multi_thread)){
					printf("Couldn't sort exchange pairings!");
					briefing->execution_successful = 0;
					return 0;
				}

				if(sorted_output_multi_thread)
					sorted_output_multi_thread_needs_free = 1;
				else
					sorted_output_multi_thread = cd.exchange_ranking + chunk_edge_start;

				wtime_iteration_edge_sorting_end = omp_get_wtime();

				// -------------------------------- SEQUENTIAL SECTION

#ifndef VERBOSE_TABLE
				if(settings->verbose) {
					printf("Potential node transfers sorting complete.\n\n");
					printf("Starting  potential node transfers selection...\n");
				}
#endif

				// Do the sequential selection of the community pairings (iterate sequentially over the sorted ranking edges)

				wtime_iteration_edge_selection_begin = omp_get_wtime();

				if(!sequential_select_pairings(&cd, sorted_output_multi_thread, total_exchanges, &selected, &stop_scanning_position)) {
					printf("Couldn't select exchange pairings!");
					briefing->execution_successful = 0;
					return 0;
				}

				wtime_iteration_edge_selection_end = omp_get_wtime();

#ifndef VERBOSE_TABLE
				if(settings->verbose) {
					printf("Potential node transfers selection complete.\n\n");
					printf("Starting execution of node transfers...\n");
				}
#endif

				// -------------------------------- END OF SEQUENTIAL SECTION

				// Parallel updates for the communities

				wtime_iteration_edge_apply_begin = omp_get_wtime();

				// TODO Parallel for
#pragma omp parallel for schedule(dynamic,PARALLEL_SORT_SELECT_CHUNKS_EXCHANGE_CHUNK_SIZE) default(shared) private(i)
				for(i = 0; i < stop_scanning_position; i++)
					if(*(selected + i))
						apply_transfer_weighted(dwg,&cd,sorted_output_multi_thread+i);

				wtime_iteration_edge_apply_end = omp_get_wtime();

#ifndef VERBOSE_TABLE
				if(settings->verbose) {
					printf("Execution of node transfers selection complete.\n\n");
				}
#endif

				// Free memory used in the last computations
				free(selected);
				if(sorted_output_multi_thread_needs_free)
					free(sorted_output_multi_thread);

				// TODO End of substitute



			} else {
				final_iteration_modularity = initial_iteration_modularity;

				wtime_iteration_edge_compression_begin = 0;
				wtime_iteration_edge_compression_end = 0;
				wtime_iteration_edge_sorting_begin = 0;
				wtime_iteration_edge_sorting_end = 0;
				wtime_iteration_edge_selection_begin = 0;
				wtime_iteration_edge_selection_end = 0;
				wtime_iteration_edge_apply_begin = 0;
				wtime_iteration_edge_apply_end = 0;
			}

		}

	// End of chunk


		wtime_iteration_end =  omp_get_wtime();

		final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_parallel(&cd);

		if(final_iteration_modularity > MAXIMUM_LEGAL_MODULARITY) {
			printf("Iteration modularity (%f) exceeding maximum legal modularity value of %f ==> Something went wrong!\n", final_iteration_modularity, MAXIMUM_LEGAL_MODULARITY);
			briefing->execution_successful = 0;
			return 0;
		}


#ifndef VERBOSE_TABLE
		if(settings->verbose) {
			printf("End of Iteration #%d.\n", phase_iteration_counter);
			printf("Final iteration modularity: %f. Modularity gain: %f", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);
		}
#endif


		wtime_iteration_node_scan = wtime_iteration_node_scan_end- wtime_iteration_node_scan_begin;
		wtime_iteration_edge_compression = wtime_iteration_edge_compression_end - wtime_iteration_edge_compression_begin;
		wtime_iteration_edge_sorting = wtime_iteration_edge_sorting_end - wtime_iteration_edge_sorting_begin;
		wtime_iteration_edge_selection = wtime_iteration_edge_selection_end - wtime_iteration_edge_selection_begin;
		wtime_iteration_edge_apply = wtime_iteration_edge_apply_end - wtime_iteration_edge_apply_begin;
		wtime_iteration = wtime_iteration_end - wtime_iteration_begin;

		if(full_iteration)
			wtime_iteration_duration_avg = merge_average(wtime_iteration_duration_avg, phase_iteration_counter,wtime_iteration, 1);
		else if(wtime_iteration_duration_avg == 0)
			wtime_iteration_duration_avg = wtime_iteration;

#ifdef VERBOSE_TABLE
	if(settings->verbose) {

		printf(PRINTING_SORT_SELECT_TIME_TABLE_VALUES_PERCENT,
				phase_iteration_counter,
				initial_iteration_modularity, final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity,
				100 * wtime_iteration_node_scan / wtime_iteration,
				100 * wtime_iteration_edge_compression / wtime_iteration,
				100 * wtime_iteration_edge_sorting / wtime_iteration,
				100 * wtime_iteration_edge_selection / wtime_iteration,
				100 * wtime_iteration_edge_apply / wtime_iteration,
				wtime_iteration);

		printf(PRINTING_SORT_SELECT_TIME_TABLE_VALUES,
				wtime_iteration_node_scan,
				wtime_iteration_edge_compression_end - wtime_iteration_edge_compression_begin,
				wtime_iteration_edge_sorting_end - wtime_iteration_edge_sorting_begin,
				wtime_iteration_edge_selection_end - wtime_iteration_edge_selection_begin,
				wtime_iteration_edge_apply_end - wtime_iteration_edge_apply_begin,
				wtime_iteration_end - wtime_iteration_begin);

	}
#endif

		phase_iteration_counter++;

	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);

	output_translator_weighted(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

	// New communities graph and results should be passed
	// TODO

	if(settings->verbose) {
		printf(PRINTING_UTILITY_SPARSE_DASHES);
		printf("End of phase\n\n");
		printf("\tNumber of iterations:           %d\n", phase_iteration_counter);
		printf("\tAverage iteration duration:     %f\n", wtime_iteration_duration_avg);
		printf("\tFinal modularity:               %f\n", final_phase_modularity);
		printf("\tModularity gain:                %f\n", final_phase_modularity - initial_phase_modularity);
	}

	briefing->execution_successful = 1;
	briefing->number_of_iterations = phase_iteration_counter;
	briefing->output_modularity = final_phase_modularity;
	briefing->average_iteration_duration = wtime_iteration_duration_avg;

	return 1;
}
