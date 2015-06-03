#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "community-development.h"
#include "sorted-linked-list.h"
#include "utilities.h"
#include <stdlib.h>
#include <omp.h>

// Needs to be >= 0 otherwise the algorithm can't converge!
#define MINIMUM_TRANSFER_GAIN 0
#define ILLEGAL_MODULARITY_VALUE -2

void community_developer_free(community_developer *cd) {
	if(cd->vertex_community)
		free(cd->vertex_community);

	if(cd->internal_weight_community)
		free(cd->internal_weight_community);
	if(cd->incoming_weight_community)
		free(cd->incoming_weight_community);
	if(cd->incoming_weight_node)
		free(cd->incoming_weight_node);

	if(cd->cumulative_edge_number)
		free(cd->cumulative_edge_number);
	if(cd->vertex_neighbor_communities)
		free(cd->vertex_neighbor_communities);
	if(cd->exchange_ranking)
		free(cd->exchange_ranking);
}

// Designed to be called at the start of each phase
int community_developer_init(community_developer *cd, dynamic_graph *dg) {
	int i;
	int accumulated_edge_number;
	int node_degree;

	cd->n = dg->size;
	cd->double_m = 0;

	if(cd->vertex_community = (int *) malloc(cd->n * sizeof(int))){
		if(cd->internal_weight_community = (int *) malloc(cd->n * sizeof(int))){
			if(cd->incoming_weight_community = (int *) malloc(cd->n * sizeof(int))){
				if(cd->incoming_weight_node = (int *) malloc(cd->n * sizeof(int))){
					if(cd->cumulative_edge_number = (int *) malloc(cd->n * sizeof(int))){
						if(cd->vertex_neighbor_communities = (int *) malloc(cd->n * sizeof(int))){
							accumulated_edge_number = 0;

							for(i = 0; i < cd->n; i++) {
								// Put each node in an individual community
								*(cd->vertex_community+i) = i;

								*(cd->internal_weight_community+i) = (dg->edges+i)->self_loop;
								cd->double_m += (dg->edges+i)->self_loop;

								node_degree = dynamic_graph_node_degree(dg, i);
								*(cd->incoming_weight_community+i) = *(cd->incoming_weight_node+i) = node_degree;
								cd->double_m += node_degree;

								accumulated_edge_number += (dg->edges+i)->count;
								*(cd->cumulative_edge_number+i) = accumulated_edge_number;

								// We don't need to initialize vertex_neighbor_communities since its content will be written when iterating over each node in each single phase iteration
							}

							if(!(cd->exchange_ranking = (community_exchange *) malloc(accumulated_edge_number * sizeof(community_exchange)))){
								printf("Out of memory!\n");
								community_developer_free(cd);

								return 0;
							}
						} else {
							printf("Out of memory!\n");
							community_developer_free(cd);

							return 0;
						}
					} else {
						printf("Out of memory!\n");
						community_developer_free(cd);

						return 0;
					}
				} else {
					printf("Out of memory!\n");
					community_developer_free(cd);

					return 0;
				}
			} else {
				printf("Out of memory!\n");
				community_developer_free(cd);

				return 0;
			}
		} else {
			printf("Out of memory!\n");
			community_developer_free(cd);

			return 0;
		}
	} else {
		printf("Out of memory!\n");

		return 0;
	}

	return 1;
}

// Designed to be called at the start of each phase
int community_developer_init_weighted(community_developer *cd, dynamic_weighted_graph *dwg) {
	int i;
	int accumulated_edge_number;
	int node_degree;

	cd->n = dwg->size;
	cd->double_m = 0;

	if(cd->vertex_community = (int *) malloc(cd->n * sizeof(int))){
		if(cd->internal_weight_community = (int *) malloc(cd->n * sizeof(int))){
			if(cd->incoming_weight_community = (int *) malloc(cd->n * sizeof(int))){
				if(cd->incoming_weight_node = (int *) malloc(cd->n * sizeof(int))){
					if(cd->cumulative_edge_number = (int *) malloc(cd->n * sizeof(int))){
						accumulated_edge_number = 0;

						for(i = 0; i < cd->n; i++) {
							// Put each node in an individual community
							*(cd->vertex_community+i) = i;
							*(cd->internal_weight_community+i) = (dwg->edges+i)->self_loop;
							cd->double_m += (dwg->edges+i)->self_loop;

							node_degree = dynamic_weighted_graph_node_degree(dwg, i);
							*(cd->incoming_weight_community+i) = *(cd->incoming_weight_node+i) = node_degree;
							cd->double_m += node_degree;

							accumulated_edge_number += (dwg->edges+i)->count;
							*(cd->cumulative_edge_number+i) = accumulated_edge_number;
						}

						if(!(cd->exchange_ranking = (community_exchange *) malloc(accumulated_edge_number * sizeof(community_exchange)))){
							printf("Out of memory!\n");
							community_developer_free(cd);

							return 0;
						}
					} else {
						printf("Out of memory!\n");
						community_developer_free(cd);

						return 0;
					}
				} else {
					printf("Out of memory!\n");
					community_developer_free(cd);

					return 0;
				}
			} else {
				printf("Out of memory!\n");
				community_developer_free(cd);

				return 0;
			}
		} else {
			printf("Out of memory!\n");
			community_developer_free(cd);

			return 0;
		}
	} else {
		printf("Out of memory!\n");

		return 0;
	}

	return 1;
}

inline int get_modularity_computing_package(modularity_computing_package *mcp,community_developer *cd, int node_index, int community_index, int k_i_in) {
	mcp->k_i = *(cd->incoming_weight_node+node_index);
	mcp->k_i_in = k_i_in;
	mcp->sum_in = *(cd->internal_weight_community+community_index);
	mcp->sum_tot = *(cd->incoming_weight_community+community_index);
	mcp->double_m = cd->double_m;
}

inline float modularity_delta_unpackaged(int sum_in, int double_m, int sum_tot, int k_i, int k_i_in) {
	float res;
	float a = ((float) (sum_tot + k_i));

	res = ((float)(sum_in + 2 * k_i_in)) - a * a /double_m;

	a = ((float) sum_tot);

	res -= ((float) sum_in - a * a / double_m - ((float) k_i * k_i) / double_m);

	res /= double_m;

	return res;
}

inline float modularity_delta(modularity_computing_package *mcp) {
	return modularity_delta_unpackaged(mcp->sum_in, mcp->double_m, mcp->sum_tot, mcp->k_i, mcp->k_i_in);
}

float removal_modularity_loss(dynamic_graph *dwg, community_developer *cd, int node_index, int k_i_in) {
	// I assume the removal loss is equal to the gain that we would get by
	// putting the node back in its former community.

	modularity_computing_package mcp;

	// TODO
	get_modularity_computing_package(&mcp, cd, node_index, *(cd->vertex_community+node_index), k_i_in);

	mcp.sum_tot -= k_i_in;
	mcp.sum_in = mcp.sum_in - 2 * k_i_in - (dwg->edges + node_index)->self_loop;

	return modularity_delta(&mcp);
}

float removal_modularity_loss_weighted(dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int k_i_in) {
	// I assume the removal loss is equal to the gain that we would get by
	// putting the node back in its former community.

	modularity_computing_package mcp;

	// TODO
	get_modularity_computing_package(&mcp, cd, node_index, *(cd->vertex_community+node_index), k_i_in);

	mcp.sum_tot -= k_i_in;
	mcp.sum_in = mcp.sum_in - 2 * k_i_in - (dwg->edges + node_index)->self_loop;

	return modularity_delta(&mcp);
}



//sorted_linked_list * get_neighbor_communities_list(sorted_linked_list *sll, dynamic_graph *dg, community_developer *cd, int node_index) {
//	dynamic_weighted_edge_array *neighbor_nodes;
//	int i;
//
//	if(!dg || !sll) {
//		printf("Invalid input in computation of neighbor communities!");
//		return 0;
//	}
//
//	if(node_index < 0 || node_index >= dg->size) {
//		printf("Invalid input in computation of neighbor communities: node out of range!");
//		return 0;
//	}
//
//	sorted_linked_list_free(sll);
//
//	neighbor_nodes = dg->edges + node_index;
//
//	//	Self loop not included in k_i_in of current community
//
//	//	if(neighbor_nodes->self_loop)
//	//		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community+node_index), neighbor_nodes->self_loop))) {
//	//			printf("Cannot insert node in sorted linked list!");
//	//
//	//			sorted_linked_list_free(sll);
//	//			free(sll);
//	//
//	//			return NULL;
//	//		}
//
//	for(i = 0; i < neighbor_nodes->count; i++) {
//		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), 1))) {
//			printf("Cannot insert node in sorted linked list!");
//
//			sorted_linked_list_free(sll);
//			free(sll);
//
//			return NULL;
//		}
//	}
//
//	return sll;
//}

// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
int get_neighbor_communities_list(sorted_linked_list *sll, dynamic_graph *dg, community_developer *cd, int node_index, int *current_community_k_i_in) {
	dynamic_edge_array *neighbor_nodes;
	int i;

	int current_community;
	int tmp_current_community_k_i_in = 0;

	if(!dg || !sll) {
		printf("Invalid input in computation of neighbor communities!");
		return 0;
	}

	if(node_index < 0 || node_index >= dg->size) {
		printf("Invalid input in computation of neighbor communities: node out of range!");
		return 0;
	}

	current_community = *(cd->vertex_community + node_index);

	sorted_linked_list_free(sll);

	neighbor_nodes = dg->edges + node_index;

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
			tmp_current_community_k_i_in += 1;
		else if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), 1))) {
			printf("Cannot insert node in sorted linked list!");

			sorted_linked_list_free(sll);
			free(sll);

			return 0;
		}
	}

	return sll;
}

// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
int get_neighbor_communities_list_weighted(sorted_linked_list *sll, dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int *current_community_k_i_in) {
	dynamic_weighted_edge_array *neighbor_nodes;
	int i;

	int current_community;
	int tmp_current_community_k_i_in = 0;

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
			tmp_current_community_k_i_in += (neighbor_nodes->addr + i)->weight;
		else if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), (neighbor_nodes->addr + i)->weight))) {
			printf("Cannot insert node in sorted linked list!");

			sorted_linked_list_free(sll);
			free(sll);

			return 0;
		}
	}

	return sll;
}

float compute_modularity(dynamic_graph *dg, community_developer *cd) {
	float result;
	int src,j;
	int dest;

	if(!dg || !cd){
		printf("Invalid input: cannot compute modularity!");
		return -2;
	}

	for(src=0;src<dg->size;src++)
		for(j=0;j<(dg->edges+src)->count;j++) {
			dest = ((dg->edges+src)->addr+j)->dest;
			result += ((float) 1 - (float) *(cd->incoming_weight_node+src) * *(cd->incoming_weight_node+dest) / cd->double_m) * same(*(cd->vertex_community+src),*(cd->vertex_community+dest));
		}

	result /= cd->double_m;

	return result;
}

float compute_modularity_weighted(dynamic_weighted_graph *dwg, community_developer *cd) {
	float result;
	int src,j;
	int dest, weight;
	
	if(!dwg || !cd){
		printf("Invalid input: cannot compute modularity!");
		return -2;
	}
	
	for(src=0;src<dwg->size;src++)
		for(j=0;j<(dwg->edges+src)->count;j++) {
			dest = ((dwg->edges+src)->addr+j)->dest;
			weight = ((dwg->edges+src)->addr+j)->weight;
			result += ((float) weight - (float) *(cd->incoming_weight_node+src) * *(cd->incoming_weight_node+dest) / cd->double_m) * same(*(cd->vertex_community+src),*(cd->vertex_community+dest));
		}
			
	result /= cd->double_m;
			
	return result;
}

inline void set_exchange_ranking(community_exchange *ce, int node, int dest, int k_i_in_src, int k_i_in_dest, float modularity_delta) {
	ce->node = node;
	ce->dest = dest;
	ce->k_i_in_src = k_i_in_src;
	ce->k_i_in_dest = k_i_in_dest;
	ce->modularity_delta = modularity_delta;
}



int sequential_select_pairings(community_developer *cd, community_exchange *exchange_rankings_sorted, int exchange_rankings_number, short *selected, int *stop_scanning_position) {
	int used_communities = 0;
	int *used_community;
	int current = 0;

	*stop_scanning_position = exchange_rankings_number;

	if(selected = (short*) malloc(exchange_rankings_number * sizeof(short))) {
		if(used_community = (int*) malloc(cd->n * sizeof(int))) {
			while(used_communities < cd->n && current < exchange_rankings_number) {
				if(!(*(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node))) && !(*(used_community + (exchange_rankings_sorted+current)->dest))) {
					// Source and destination communities weren't used yet by any exchange

					// Exchange selected
					*(selected + current) = 1;
					// Source community used
					*(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node)) = 1;
					// Destination community used
					*(used_community + (exchange_rankings_sorted+current)->dest) = 1;

					used_communities += 2;

					// Last selected exchange updated. Stop scanning selected array at position current + 1.
					*stop_scanning_position = current + 1;
				} else {
					// Source and destination communities were already used by any exchange

					// Exchange selected
					*(selected + current) = 0;
				}
			}

			free(used_community);
		} else {
			printf("Cannot allocate used community array!");

			return 0;
		}
	} else {
		printf("Cannot allocate selected array!");

		return 0;
	}

	return 1;
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
}

int community_exchange_compare (const void * a, const void * b)
{
   return ( ((community_exchange*)a)->modularity_delta - ((community_exchange*)b)->modularity_delta );
}

int lower_power_of_2(int n) {
	int result;

	if(n <= 0)
		return -1;

	result = 1;

	while(result * 2 <= n) {
		result *= 2;
	}

	return result;
}

void community_exchange_parallel_quick_sort_merge(community_exchange *exchange_rankings, int first_partition_start, int first_partition_end, int second_partition_start, int second_partition_end,community_exchange *output_sorted) {
	int i = first_partition_start;
	int j = second_partition_start;

	int compare_result;
	int copy_to = first_partition_start;

	while(i < first_partition_end && j < second_partition_end) {
		compare_result = community_exchange_compare(exchange_rankings+i, exchange_rankings+j);

		if(compare_result > 0) {
			// First greater then second
			*(output_sorted+copy_to) = *(exchange_rankings+i);

			i++;
		} else if (compare_result == 0) {
			// First and second are equal
			*(output_sorted+copy_to) = *(exchange_rankings+i);
			copy_to++;
			*(output_sorted+copy_to) = *(exchange_rankings+i);

			i++;
			j++;

		} else {
			// Second greater than first
			*(output_sorted+copy_to) = *(exchange_rankings+j);

			j++;
		}

		copy_to++;
	}

	// If first partition was not fully copied
	for(;i < first_partition_end;i++) {
		*(output_sorted+copy_to) = *(exchange_rankings+i);
		copy_to++;
	}

	// If second partition was not fully copied
	for(;j < second_partition_end;j++) {
		*(output_sorted+copy_to) = *(exchange_rankings+j);
		copy_to++;
	}
}

int community_exchange_parallel_quick_sort_main(community_exchange *exchange_rankings, int total_exchanges, int number_of_threads, community_exchange **output_sorted) {
	int i;
	int partitions_number = number_of_threads;
	int partition_size;
	int base_partition_size;

	int first_partition_start;
	int first_partition_end;
	int second_partition_start;
	int second_partition_end;

	output_sorted = NULL;

	partitions_number = lower_power_of_2(number_of_threads);

	base_partition_size = total_exchanges / partitions_number;

	// Divide the input array in partition and qsort them in parallel
	#pragma omp parallel for scheduling(static, 1) num_treads(partitions_number)
	for(i = 0; i < partitions_number; i++)
	{

		if(i == (number_of_threads - 1))
			partition_size = total_exchanges - base_partition_size * partitions_number;
		else
			partition_size = base_partition_size;

		qsort(exchange_rankings + i, partition_size, sizeof(community_exchange), community_exchange_compare);
	}

	if(partitions_number > 1 && !(*output_sorted = (community_exchange *) malloc(total_exchanges * sizeof(community_exchange)))) {
		printf("Couldn't allocate space needed to merge partitions in sorting of community exchanges!");

		return 0;
	}

	while(partitions_number != 1) {

		// Merge subsections of the array in parallel
		#pragma omp parallel for scheduling(static, 1) num_treads(partitions_number)
		for(i = 0; i < partitions_number; i+=2) {

			first_partition_start = i * partition_size;
			second_partition_start = first_partition_end = (i + 1) * partition_size;

			if(i == (partitions_number - 2))
				second_partition_end = total_exchanges - partitions_number * base_partition_size;
			else
				second_partition_end = (i + 2) * partition_size;

			community_exchange_parallel_quick_sort_merge(exchange_rankings, first_partition_start, first_partition_end, second_partition_start, second_partition_end, *output_sorted);
		}

		partitions_number /= 2;
		base_partition_size *= 2;
	}

	return 1;
}

typedef struct temporary_community_edge {
	int self_loop;
	sorted_linked_list sll;
} temporary_community_edge;

void temporary_community_edge_init(temporary_community_edge *tce){
	tce->self_loop = 0;
}

void temporary_community_edge_free(temporary_community_edge *tce){
	sorted_linked_list_free(&(tce->sll));
}

int output_translator(dynamic_weighted_graph *dwg, community_developer *cd, dynamic_weighted_graph *community_graph, int **community_vector) {

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

	if(!(community_graph = (dynamic_weighted_graph *) malloc (sizeof(dynamic_weighted_graph)))) {
		printf("Cannot allocate output community graph!");

		return 0;
	}

	if(!(*community_vector = (int *) malloc (cd->n * sizeof(int)))) {
		printf("Cannot allocate non_empty_community community graph!");

		free(community_graph);
		return 0;
	}

	if(!(renumber_community = (int *) malloc (cd->n * sizeof(int)))) {
		printf("Cannot allocate renumber_community community graph!");

		free(community_graph);
		free(*community_vector);
		return 0;
	}

	#pragma omp parallel for scheduling(dynamic,10) shared(non_empty_community) private(i)
	for(i = 0; i < cd->n ; i++)
		*(renumber_community+i) = -1;

	#pragma omp parallel for scheduling(dynamic,10) shared(non_empty_community) private(i, community)
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

	dynamic_weighted_graph_init(community_graph, total_communities);

	if(!(tce = (temporary_community_edge*) malloc(total_communities * sizeof(temporary_community_edge)))) {
		printf("Cannot allocate temporary_community_edge community graph!");

		free(community_graph);
		free(*community_vector);
		return 0;
	}

	#pragma omp parallel for schedule(dynamic,10) private(i)
	for(i = 0; i < total_communities; i++)
		temporary_community_edge_init(tce+i);

	// -------------------------------- SEQUENTIAL SECTION

	for(current_node = 0; current_node < cd->n ; current_node++) {
		neighbors = *(dwg->edges + current_node);
		community_of_current_node = *(*community_vector+current_node);

		for(j = 0; j < neighbors.count; j++){
			neighbor = *(neighbors.addr+j);

			if(*(*community_vector+neighbor.dest) == community_of_current_node)
				// Community selfloop
				(tce+community_of_current_node)->self_loop += neighbor.weight;
			else
				sorted_linked_list_insert(&((tce+community_of_current_node)->sll), *(*community_vector+neighbor.dest), neighbor.weight);
		}
	}

	#pragma omp parallel for schedule(dynamic,5) private(i)
	for(i = 0; i < total_communities; i++) {
		(community_graph->edges + i)->self_loop = (tce+i)->self_loop;

		neighbor_community = ((tce+i)->sll).head;
		while(neighbor_community) {
			dynamic_weighted_graph_insert(community_graph, i, neighbor_community->community, neighbor_community->k_i_in);

			neighbor_community = neighbor_community->next;
		}
	}

	temporary_community_edge_free(tce);
	free(tce);

	return 1;
}

// Executes a phase of the algorithm, returns modularity gain
float phase_weighted(dynamic_weighted_graph *dwg, float minimum_improvement, int num_threads, dynamic_weighted_graph *community_graph, int **community_vector) {
	community_developer cd;

	float initial_phase_modularity, final_phase_modularity;
	float initial_iteration_modularity, final_iteration_modularity;

	sorted_linked_list neighbors;
	sorted_linked_list_elem *neighbor;
	modularity_computing_package mcp;
	float to_neighbor_modularity_delta;
	int current_community_k_i_in;
	float removal_loss;
	float gain;
	// Number of neighbor communities of a node worth considering for potential node transfer
	int number_of_neighbor_communities;

	int base,total_exchanges;

	community_exchange *sorted_output_multi_thread;
	// True if it is actually used (multi-threading)
	int sorted_output_multi_thread_needs_free;

	short *selected;
	int stop_scanning_position;

	int i,j;

	if(!dwg || minimum_improvement < 0 || minimum_improvement > 2) {
		printf("Invalid phase parameters!");

		return ILLEGAL_MODULARITY_VALUE;
	}


	community_developer_init_weighted(&cd, dwg);

	initial_phase_modularity = compute_modularity_weighted(dwg, &cd);
	final_iteration_modularity = initial_phase_modularity;

	do {
		sorted_output_multi_thread_needs_free = 0;

		initial_iteration_modularity = final_iteration_modularity;

		// Do the parallel iterations over all nodes
		// TODO put Parallel for - Everything else is good to go
		for(i = 0; i < dwg->size; i++) {
			// Node i, current edges: dwg->edges + i

			number_of_neighbor_communities = 0;

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
				to_neighbor_modularity_delta = modularity_delta(get_modularity_computing_package(&mcp,&cd,i,neighbor->community,neighbor->k_i_in));

				// Check if the gain is worth to be considered
				gain = to_neighbor_modularity_delta - removal_loss;
				if(gain > MINIMUM_TRANSFER_GAIN) {
					// Put computed potential transfer in its correct position for later community pairing selection
					set_exchange_ranking((cd.exchange_ranking + *(cd.cumulative_edge_number + i) + number_of_neighbor_communities),i,neighbor->community,current_community_k_i_in, neighbor->k_i_in,gain);

					number_of_neighbor_communities += 1;
				}

				neighbor = neighbor->next;
			}

			sorted_linked_list_free(&neighbors);

			*(cd.vertex_neighbor_communities + i) = number_of_neighbor_communities;
		}

		// -------------------------------- SEQUENTIAL SECTION

		// Start copying in the first free position
		base = 0;
		total_exchanges = 0;
		do {
			total_exchanges += *(cd.vertex_neighbor_communities+base);
			base++;
		}while(base < dwg->size && *(cd.cumulative_edge_number+base) == total_exchanges);

		for(i = base; i < dwg->size; i++)
			for(j = 0; j < *(cd.vertex_neighbor_communities + i); j++) {
				*(cd.exchange_ranking + total_exchanges) = *(cd.exchange_ranking + *(cd.cumulative_edge_number + base) + j);

				total_exchanges++;
			}

		// total_exchanges now represents the total number of active exchanges to be sorted

		// -------------------------------- END OF SEQUENTIAL SECTION

		// Do the parallel sorting of the community pairings array

		if(!community_exchange_parallel_quick_sort_main(cd.exchange_ranking, total_exchanges, num_threads, &sorted_output_multi_thread)){
			printf("Couldn't sort exchange pairings!");

			return ILLEGAL_MODULARITY_VALUE;
		}

		if(sorted_output_multi_thread)
			sorted_output_multi_thread_needs_free = 1;
		else
			sorted_output_multi_thread = cd.exchange_ranking;

		// -------------------------------- SEQUENTIAL SECTION

		// Do the sequential selection of the community pairings (iterate sequentially over the sorted ranking edges)

		if(!sequential_select_pairings(&cd, sorted_output_multi_thread, total_exchanges, selected, &stop_scanning_position)) {
			printf("Couldn't select exchange pairings!");

			return ILLEGAL_MODULARITY_VALUE;
		}

		// -------------------------------- END OF SEQUENTIAL SECTION

		// Parallel updates for the communities

		// TODO Parallel for
		for(i = 0; i < total_exchanges; i++)
			if(*(selected + i))
				apply_transfer_weighted(dwg,&cd,sorted_output_multi_thread+i);

		// Free memory used in the last computations
		free(selected);
		if(sorted_output_multi_thread_needs_free)
			free(sorted_output_multi_thread);

		// TODO End of substitute

		final_iteration_modularity = compute_modularity_weighted(dwg, &cd);

	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);

	output_translator(dwg, &cd, community_graph, community_vector);

	community_developer_free(&cd);

	final_phase_modularity = final_iteration_modularity;

	// New communities graph and results should be passed
	// TODO

	return final_phase_modularity - initial_phase_modularity;
}
