/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "community-computation-weighted.h"
#include "temporary-community-edge.h"
#include "community-computation-commons.h"

#include "community-development.h"
#include "dynamic-weighted-graph.h"
#include "community-exchange.h"
#include "silent-switch.h"
#include "sorted-linked-list.h"
#include "utilities.h"

#include "execution-settings.h"
#include "execution-briefing.h"

#include "printing_controller.h"

#include <time.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef SILENT_SWITCH_COMMUNITY_COMPUTATION_ON
#define printf(...)
#endif

#define NODE_ITERATION_CHUNK_SIZE 500
#define EXCHANGE_CHUNK_SIZE 60




double removal_modularity_loss_weighted(dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int k_i_in) {
	// I assume the removal loss is equal to the gain that we would get by
	// putting the node back in its former community.

	modularity_computing_package mcp;

	get_modularity_computing_package(&mcp, cd, node_index, *(cd->vertex_community+node_index), k_i_in);

//	// TODO Fix potential problem with self loops
//	mcp.sum_tot -= k_i_in;
	// Fix
	mcp.sum_tot -= *(cd->incoming_weight_node + node_index);
	mcp.sum_in = mcp.sum_in - 2 * k_i_in - (dwg->edges + node_index)->self_loop;

	return modularity_delta(&mcp);
}

// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
int get_neighbor_communities_list_weighted(sorted_linked_list *sll, dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int *current_community_k_i_in) {
	dynamic_weighted_edge_array *neighbor_nodes;
	int i;

	int current_community;
	*current_community_k_i_in = 0;

	if(!dwg || !sll) {
		printf("Invalid input in computation of neighbor communities!");
		return 0;
	}

	if(node_index < 0 || node_index >= dwg->size) {
		printf("Invalid input in computation of neighbor communities: node out of range!");
		return 0;
	}

	current_community = *(cd->vertex_community + node_index);

	sorted_linked_list_free(sll);

	neighbor_nodes = dwg->edges + node_index;

//	Self loop not included in k_i_in of current community

//	if(neighbor_nodes->self_loop)
//		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community+node_index), neighbor_nodes->self_loop))) {
//			printf("Cannot insert node in sorted linked list!");
//
//			sorted_linked_list_free(sll);
//			free(sll);
//
//			return NULL;
//		}

	for(i = 0; i < neighbor_nodes->count; i++) {
		if(*(cd->vertex_community + (neighbor_nodes->addr + i)->dest) == current_community)
			// Link internal to current node community
			*current_community_k_i_in += (neighbor_nodes->addr + i)->weight;
		else if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), (neighbor_nodes->addr + i)->weight))) {
			printf("Cannot insert node in sorted linked list!");

			sorted_linked_list_free(sll);
			free(sll);

			return 0;
		}
	}

	return 1;
}

double compute_modularity_edge_weighted(dynamic_weighted_graph *dwg, int edge_weight, int src_incoming_weight, int dest_incoming_weight, int double_m, int src_community, int dest_community) {
	return ((double) edge_weight - ((((double) src_incoming_weight) * ((double) dest_incoming_weight)) / ((double) double_m)) ) * same(src_community,dest_community);
//	return ((double) edge_weight - (double) src_incoming_weight * dest_incoming_weight / double_m) * same(src_community,dest_community);
}

double compute_modularity_weighted_reference_implementation_method(community_developer *cd) {
	int i;
	double internal_weight_community;
	double incoming_weight_community;
	double double_m = (double) cd->double_m;

	double result = 0;

	for(i = 0; i < cd->n; i++) {
		internal_weight_community = (double)*(cd->internal_weight_community + i);
		incoming_weight_community = (double) *(cd->incoming_weight_community + i);

		result += (internal_weight_community - incoming_weight_community * incoming_weight_community / double_m);
	}

	result /= double_m;

	return result;
}

double compute_modularity_weighted_reference_implementation_method_parallel(community_developer *cd) {
	int i;
	double internal_weight_community;
	double incoming_weight_community;
	double double_m = (double) cd->double_m;

	double result = 0;

#pragma omp parallel for schedule(dynamic, 20) default(shared) private(internal_weight_community, incoming_weight_community) reduction(+:result)
	for(i = 0; i < cd->n; i++) {
		internal_weight_community = (double)*(cd->internal_weight_community + i);
		incoming_weight_community = (double) *(cd->incoming_weight_community + i);

		result += (internal_weight_community - incoming_weight_community * incoming_weight_community / double_m);
	}

	result /= double_m;

	return result;
}

double compute_modularity_weighted_reference_implementation_method_range(community_developer *cd, int start, int end) {
	int i;
	double internal_weight_community;
	double incoming_weight_community;
	double double_m = (double) cd->double_m;

	double result = 0;

	if(!in_range(start,0,cd->n-1) || !in_range(end, start, cd->n-1)) {
		printf("compute_modularity_weighted_reference_implementation_method_range - Given range is not valid (%d - %d)!", start, end);

		return ILLEGAL_MODULARITY_VALUE;
	}

	for(i = start; i < end; i++) {
		internal_weight_community = (double)*(cd->internal_weight_community + i);
		incoming_weight_community = (double) *(cd->incoming_weight_community + i);

		result += (internal_weight_community - incoming_weight_community * incoming_weight_community / double_m);

//		result += ((double)*(cd->internal_weight_community + i) - (  (((double) *(cd->incoming_weight_community + i)) * ((double) *(cd->incoming_weight_community + i))) /((double) cd->double_m) )      );
	}

	result /= double_m;

//	result /= ((double) cd->double_m);

	return result;
}

// Computes modularity as if each node was in an individual community
double compute_modularity_init_weighted_reference_implementation_method(dynamic_weighted_graph *dwg) {
	int i;

	double result = 0;

	double internal_weight_community, incoming_weight_community, double_m;

	double_m = dynamic_weighted_graph_double_m(dwg);

	for(i = 0; i < dwg->size; i++) {
		internal_weight_community = (double) dynamic_weighted_graph_self_loop(dwg, i);
		incoming_weight_community = (double) dynamic_weighted_graph_node_degree(dwg, i);

		result += (internal_weight_community - incoming_weight_community * incoming_weight_community / double_m );
	}

	result /= double_m;

	return result;
}

inline void apply_transfer_weighted(dynamic_weighted_graph *dwg, community_developer *cd, community_exchange *exchange) {
	const int src =  *(cd->vertex_community + exchange->node);
	const int self_loop =  (dwg->edges + exchange->node)->self_loop;

	// Update source community sum_tot (incoming_weight_community)
	*(cd->incoming_weight_community + src) -= *(cd->incoming_weight_node + exchange->node);
	// Update source community sum_int (internal_weight_community)
	*(cd->internal_weight_community + src) -= (2 * exchange->k_i_in_src + self_loop);
	// Update destination community sum_tot (incoming_weight_community)
	*(cd->incoming_weight_community + exchange->dest) += *(cd->incoming_weight_node + exchange->node);
	// Update destination community sum_int (internal_weight_community)
	*(cd->internal_weight_community + exchange->dest) += (2 * exchange->k_i_in_dest + self_loop);

	*(cd->vertex_community + exchange->node) = exchange->dest;

//	printf("Moving vertex %d from community %d to community %d.\n", exchange->node, src, exchange->dest);
}

int output_translator_weighted(dynamic_weighted_graph *dwg, community_developer *cd, dynamic_weighted_graph **community_graph, int **community_vector) {

	// TODO free output, just in case

	int *renumber_community;
	int i,j;

	int current_node;

	int community_of_current_node;

	int total_communities = 0;
	int community;

	temporary_community_edge *tce;

	dynamic_weighted_edge_array neighbors;

	weighted_edge neighbor;

	sorted_linked_list_elem *neighbor_community;

//	printf("\n\n------------- OUTPUT TRANSLATION ------------\n\n");
//
//	printf("Input graph:\n\n");

//	dynamic_weighted_graph_print(*dwg);

//	printf("\n\nInput CD:\n\n");

	community_developer_print(cd,0);

//	printf("Output translation - Final modularity of input (reference formula): %f\n\n", compute_modularity_weighted_reference_implementation_method(cd));

	if(!(*community_graph = (dynamic_weighted_graph *) malloc (sizeof(dynamic_weighted_graph)))) {
		printf("Cannot allocate output community graph!");

		return 0;
	}

	if(!(*community_vector = (int *) malloc (cd->n * sizeof(int)))) {
		printf("Cannot allocate non_empty_community community graph!");

		free(*community_graph);
		return 0;
	}

	if(!(renumber_community = (int *) malloc (cd->n * sizeof(int)))) {
		printf("Cannot allocate renumber_community community graph!");

		free(*community_graph);
		free(*community_vector);
		return 0;
	}

#pragma omp parallel for default(shared) private(i) schedule(dynamic,50)
	for(i = 0; i < cd->n ; i++)
		*(renumber_community+i) = -1;

//	#pragma omp parallel for scheduling(dynamic,10) shared(non_empty_community) private(i, community)
	for(i = 0; i < cd->n ; i++) {
		community = *(cd->vertex_community + i);

		if(*(renumber_community+community) != -1)
			// Community is already re-numbered
			*(*community_vector+i) = *(renumber_community+community);
		else {
			// Community isn't re-numbered yet
			*(renumber_community+community) = total_communities;
			*(*community_vector+i) = total_communities;

			total_communities++;
		}
	}

	free(renumber_community);

	dynamic_weighted_graph_init(*community_graph, total_communities);

	if(!(tce = (temporary_community_edge*) malloc(total_communities * sizeof(temporary_community_edge)))) {
		printf("Cannot allocate temporary_community_edge community graph!");

		free(*community_graph);
		free(*community_vector);
		return 0;
	}

	#pragma omp parallel for schedule(dynamic,50) default(shared) private(i)
	for(i = 0; i < total_communities; i++)
		temporary_community_edge_init(tce+i);

	// -------------------------------- SEQUENTIAL SECTION

	for(current_node = 0; current_node < cd->n ; current_node++) {
		neighbors = *(dwg->edges + current_node);
		community_of_current_node = *(*community_vector+current_node);

//		// TODO Review this instruction
		(tce+community_of_current_node)->self_loop += (dwg->edges + current_node)->self_loop;

		for(j = 0; j < neighbors.count; j++){
			neighbor = *(neighbors.addr+j);

			if(*(*community_vector+neighbor.dest) == community_of_current_node)
				// Community selfloop
				(tce+community_of_current_node)->self_loop += neighbor.weight;
			else
				sorted_linked_list_insert(&((tce+community_of_current_node)->sll), *(*community_vector+neighbor.dest), neighbor.weight);
		}
	}

	// -------------------------------- END OF SEQUENTIAL SECTION

//	#pragma omp parallel for schedule(dynamic,10) default(shared) private(i)
	for(i = 0; i < total_communities; i++) {
		((*community_graph)->edges + i)->self_loop = (tce+i)->self_loop;

		neighbor_community = ((tce+i)->sll).head;
		while(neighbor_community) {
			// Force directed is needed to avoid having duplicated edges
			if(!(dynamic_weighted_graph_insert_force_directed(*community_graph, i, neighbor_community->community, neighbor_community->k_i_in))) {
				printf("Output translation - Could not insert edge %d - %d", i, neighbor_community->community);

				temporary_community_edge_free(tce);
				dynamic_weighted_graph_free(*community_graph);
				free(*community_graph);
				free(*community_vector);
				free(tce);

				return 0;
			}

			neighbor_community = neighbor_community->next;
		}
	}

	temporary_community_edge_free(tce);
	free(tce);

	if(!dynamic_weighted_graph_reduce(*community_graph)) {
		printf("Output translation - Couldn't reduce output graph!");

		dynamic_weighted_graph_free(*community_graph);
		free(*community_graph);
		free(*community_vector);

		return 0;
	}

	return 1;
}

// Executes a phase of the algorithm, returns modularity gain
int parallel_phase_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing) {
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



	int phase_iteration_counter = 0;

	int neighbor_communities_bad_computation;

	// Time measurements
	double wtime_phase_begin, wtime_phase_init_cd_begin, wtime_phase_init_cd_end, wtime_phase_output_translation_begin, wtime_phase_output_translation_end, wtime_phase_end;
	double wtime_iteration_begin, wtime_iteration_node_scan_begin, wtime_iteration_node_scan_end, wtime_iteration_edge_compression_begin, wtime_iteration_edge_compression_end, wtime_iteration_edge_sorting_begin, wtime_iteration_edge_sorting_end, wtime_iteration_edge_selection_begin, wtime_iteration_edge_selection_end, wtime_iteration_edge_apply_begin, wtime_iteration_edge_apply_end, wtime_iteration_end;
	int full_iteration;
	double wtime_iteration, wtime_iteration_node_scan, wtime_iteration_edge_compression, wtime_iteration_edge_sorting, wtime_iteration_edge_selection, wtime_iteration_edge_apply;
	double wtime_iteration_duration_avg;

	wtime_phase_begin = omp_get_wtime();

	// Minimum improvement refers to iteration improvement
	if(!dwg || !valid_minimum_improvement(minimum_improvement)) {
		printf("Invalid phase parameters!");

		return 0;
	}

	wtime_phase_init_cd_begin = omp_get_wtime();

	community_developer_init_weighted(&cd, dwg);

	wtime_phase_init_cd_end = omp_get_wtime();

//	community_developer_print(&cd,0);

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

		sorted_output_multi_thread_needs_free = 0;

		initial_iteration_modularity = final_iteration_modularity;

		neighbor_communities_bad_computation = 0;

#ifndef VERBOSE_TABLE

		if(settings->verbose) {
			printf(PRINTING_UTILITY_SPARSE_DASHES);

			printf(PRINTING_UTILITY_INDENT_TITLE);
			printf("Iteration #%d\n\n", phase_iteration_counter);

			printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);
			printf("Starting iteration over all nodes...\n");
		}

#endif

		total_exchanges = 0;

		wtime_iteration_node_scan_begin = omp_get_wtime();

		// Do the parallel iterations over all nodes
		// TODO put Parallel for - Everything else is good to go
#pragma omp parallel for default(shared) schedule(dynamic,NODE_ITERATION_CHUNK_SIZE) \
	private(i, node_exchanges_base_pointer, number_of_neighbor_communities,neighbors, current_community_k_i_in, removal_loss, neighbor, mcp, to_neighbor_modularity_delta, gain) \
	reduction(+:total_exchanges)
		for(i = 0; i < dwg->size; i++) {

//			printf("NODE ITERATION - Thread #%d on node %d\n", omp_get_thread_num(), i);

			if(i > 0)
				node_exchanges_base_pointer = cd.exchange_ranking + *(cd.cumulative_edge_number + i - 1);
			else
				node_exchanges_base_pointer = cd.exchange_ranking;

			// Node i, current edges: dwg->edges + i

			number_of_neighbor_communities = 0;
			sorted_linked_list_init(&neighbors);

			// Compute neighbor communities and k_i_in
			if(get_neighbor_communities_list_weighted(&neighbors,dwg,&cd,i,&current_community_k_i_in)) {


				removal_loss = removal_modularity_loss_weighted(dwg, &cd, i, current_community_k_i_in);

				neighbor = neighbors.head;
				while(neighbor){
					// Note that by the definition of get_neighbor_communities_list_weighted current community isn't in the list

					// Compute modularity variation that would be achieved my moving node i from its current community to the neighbor community considered (neighbor->community)
					get_modularity_computing_package(&mcp,&cd,i,neighbor->community,neighbor->k_i_in);
					to_neighbor_modularity_delta = modularity_delta(&mcp);

					// Check if the gain is worth to be considered
					gain = to_neighbor_modularity_delta - removal_loss;
					if(gain > MINIMUM_TRANSFER_GAIN) {
						// Put computed potential transfer in its correct position for later community pairing selection

						set_exchange_ranking(node_exchanges_base_pointer + number_of_neighbor_communities,i,neighbor->community,current_community_k_i_in, neighbor->k_i_in,gain);

						number_of_neighbor_communities += 1;

					}

					neighbor = neighbor->next;
				}

				sorted_linked_list_free(&neighbors);

				*(cd.vertex_neighbor_communities + i) = number_of_neighbor_communities;
				total_exchanges+=number_of_neighbor_communities;
			} else
#pragma omp atomic
				neighbor_communities_bad_computation++;
		}

		if(neighbor_communities_bad_computation) {
			printf("Could not compute neighbor communities!\n");

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
			base = 0;

			exchange_index = 0;

			do {
				exchange_index += *(cd.vertex_neighbor_communities+base);
				base++;
			}while(base < dwg->size && *(cd.cumulative_edge_number+base-1) == exchange_index);

			for(i = base; i < dwg->size; i++)
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

			if(!community_exchange_parallel_quick_sort_main(cd.exchange_ranking, total_exchanges, settings, &sorted_output_multi_thread)){
				printf("Couldn't sort exchange pairings!");

				return 0;
			}

			if(sorted_output_multi_thread)
				sorted_output_multi_thread_needs_free = 1;
			else
				sorted_output_multi_thread = cd.exchange_ranking;

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
#pragma omp parallel for schedule(dynamic,EXCHANGE_CHUNK_SIZE) default(shared) private(i)
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

			final_iteration_modularity = compute_modularity_weighted_reference_implementation_method_parallel(&cd);

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

		wtime_iteration_end =  omp_get_wtime();

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

