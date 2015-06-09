//#include "dynamic-graph.h"
//#include "dynamic-weighted-graph.h"
//#include "community-development.h"
//#include "sorted-linked-list.h"
//#include "utilities.h"
//#include <stdlib.h>
//#include <omp.h>
//
//// Needs to be >= 0 otherwise the algorithm can't converge!
//#define MINIMUM_TRANSFER_GAIN 0
//#define ILLEGAL_MODULARITY_VALUE -2
//
//void community_developer_free(community_developer *cd) {
//
//	if(cd->vertex_community)
//		free(cd->vertex_community);
//
//	if(cd->internal_weight_community)
//		free(cd->internal_weight_community);
//	if(cd->incoming_weight_community)
//		free(cd->incoming_weight_community);
//	if(cd->incoming_weight_node)
//		free(cd->incoming_weight_node);
//
//	if(cd->cumulative_edge_number)
//		free(cd->cumulative_edge_number);
//	if(cd->vertex_neighbor_communities)
//		free(cd->vertex_neighbor_communities);
//	if(cd->exchange_ranking)
//		free(cd->exchange_ranking);
//}
//
//void community_developer_print(community_developer *cd, int total_exchanges) {
//	int i;
//
//	printf("\n\n---------------- PRINTING COMMUNITY DEVELOPER ----------------\n\n");
//
//	printf("Number of nodes: %d\nTotal weight in graph: %d\n\n", cd->n, cd->double_m);
//
//	for(i = 0; i < cd->n; i++)
//		printf("-- Node %d:\n\nCommunity: %d\nNeighbor communities: %d\nIncoming node weight: %d\nCumulative edge number: %d\n\n ", i, *(cd->vertex_community + i), *(cd->vertex_neighbor_communities + i),*(cd->incoming_weight_node + i),*(cd->cumulative_edge_number + i));
//
//	for(i = 0; i < cd->n; i++)
//		printf("-- Community %d:\n\nIncoming weight: %d\nInternal weight: %d\n\n",i, *(cd->incoming_weight_community + i), *(cd->internal_weight_community + i));
//
//	for(i = 0; i < total_exchanges; i++)
//		printf("-- Potential exchange:\n\nNode to transfer: %d\nDestination community: %d\nSource community k_i_in: %d\nDestination community k_i_in: %d\nModularity delta: %f\n\n", (cd->exchange_ranking + i)->node, (cd->exchange_ranking + i)->dest, (cd->exchange_ranking + i)->k_i_in_src, (cd->exchange_ranking + i)->k_i_in_dest, (cd->exchange_ranking + i)->modularity_delta);
//
//	printf("\n\n------------- END OF PRINTING COMMUNITY DEVELOPER -------------\n\n");
//
//}
//
//// Designed to be called at the start of each phase
//int community_developer_init(community_developer *cd, dynamic_graph *dg) {
//	int i;
//	int accumulated_edge_number;
//	int node_degree;
//
//	cd->n = dg->size;
//	cd->double_m = 0;
//
//	if(cd->vertex_community = (int *) malloc(cd->n * sizeof(int))){
//		if(cd->internal_weight_community = (int *) malloc(cd->n * sizeof(int))){
//			if(cd->incoming_weight_community = (int *) malloc(cd->n * sizeof(int))){
//				if(cd->incoming_weight_node = (int *) malloc(cd->n * sizeof(int))){
//					if(cd->cumulative_edge_number = (int *) malloc(cd->n * sizeof(int))){
//						if(cd->vertex_neighbor_communities = (int *) malloc(cd->n * sizeof(int))){
//							accumulated_edge_number = 0;
//
//							for(i = 0; i < cd->n; i++) {
//								// Put each node in an individual community
//								*(cd->vertex_community+i) = i;
//
//								*(cd->internal_weight_community+i) = (dg->edges+i)->self_loop;
//								cd->double_m += (dg->edges+i)->self_loop;
//
//								node_degree = dynamic_graph_node_degree(dg, i);
//								*(cd->incoming_weight_community+i) = *(cd->incoming_weight_node+i) = node_degree;
//								cd->double_m += node_degree;
//
//								accumulated_edge_number += (dg->edges+i)->count;
//								*(cd->cumulative_edge_number+i) = accumulated_edge_number;
//
//								// We don't need to initialize vertex_neighbor_communities since its content will be written when iterating over each node in each single phase iteration
//							}
//
//							if(!(cd->exchange_ranking = (community_exchange *) malloc(accumulated_edge_number * sizeof(community_exchange)))){
//								printf("Out of memory!\n");
//								community_developer_free(cd);
//
//								return 0;
//							}
//						} else {
//							printf("Out of memory!\n");
//							community_developer_free(cd);
//
//							return 0;
//						}
//					} else {
//						printf("Out of memory!\n");
//						community_developer_free(cd);
//
//						return 0;
//					}
//				} else {
//					printf("Out of memory!\n");
//					community_developer_free(cd);
//
//					return 0;
//				}
//			} else {
//				printf("Out of memory!\n");
//				community_developer_free(cd);
//
//				return 0;
//			}
//		} else {
//			printf("Out of memory!\n");
//			community_developer_free(cd);
//
//			return 0;
//		}
//	} else {
//		printf("Out of memory!\n");
//
//		return 0;
//	}
//
//	return 1;
//}
//
//// Designed to be called at the start of each phase
//int community_developer_init_weighted(community_developer *cd, dynamic_weighted_graph *dwg) {
//	int i;
//	int accumulated_edge_number;
//	int node_degree;
//
//	cd->n = dwg->size;
//	cd->double_m = 0;
//
//	// Number of vertex neighbor communities doesn't need to be initialized, since it is overwritten when a node is considered
//
//	if(!(cd->vertex_community = (int *) malloc(cd->n * sizeof(int))) ||
//			!(cd->internal_weight_community = (int *) malloc(cd->n * sizeof(int))) ||
//			!(cd->incoming_weight_community = (int *) malloc(cd->n * sizeof(int))) ||
//			!(cd->incoming_weight_node = (int *) malloc(cd->n * sizeof(int))) ||
//			!(cd->cumulative_edge_number = (int *) malloc(cd->n * sizeof(int))) ||
//			!(cd->vertex_neighbor_communities = (int *) malloc(cd->n * sizeof(int)))) {
//
//		printf("Out of memory!\n");
//		community_developer_free(cd);
//
//		return 0;
//	}
//
//	accumulated_edge_number = 0;
//
//	for(i = 0; i < cd->n; i++) {
//		// Put each node in an individual community
//		*(cd->vertex_community+i) = i;
//		*(cd->internal_weight_community+i) = (dwg->edges+i)->self_loop;
//		cd->double_m += (dwg->edges+i)->self_loop;
//
//		node_degree = dynamic_weighted_graph_node_degree(dwg, i);
//		*(cd->incoming_weight_community+i) = *(cd->incoming_weight_node+i) = node_degree;
//		cd->double_m += node_degree;
//
//		accumulated_edge_number += (dwg->edges+i)->count;
//		*(cd->cumulative_edge_number+i) = accumulated_edge_number;
//	}
//
//	if(!(cd->exchange_ranking = (community_exchange *) malloc(accumulated_edge_number * sizeof(community_exchange)))){
//		printf("Out of memory!\n");
//		community_developer_free(cd);
//
//		return 0;
//	}
//
//
//	return 1;
//}
//
//
//
//inline void get_modularity_computing_package(modularity_computing_package *mcp,community_developer *cd, int node_index, int community_index, int k_i_in) {
//	mcp->k_i = *(cd->incoming_weight_node+node_index);
//	mcp->k_i_in = k_i_in;
//	mcp->sum_in = *(cd->internal_weight_community+community_index);
//	mcp->sum_tot = *(cd->incoming_weight_community+community_index);
//	mcp->double_m = cd->double_m;
//}
//
//inline double modularity_delta_unpackaged(int sum_in, int double_m, int sum_tot, int k_i, int k_i_in) {
//	double res;
//	double a = ((double) (sum_tot + k_i));
//
//	res = ((double)(sum_in + 2 * k_i_in)) - a * a /double_m;
//
//	a = ((double) sum_tot);
//
//	res -= ((double) sum_in - a * a / double_m - ((double) k_i * k_i) / double_m);
//
//	res /= double_m;
//
//	return res;
//}
//
//inline double modularity_delta(modularity_computing_package *mcp) {
//	return modularity_delta_unpackaged(mcp->sum_in, mcp->double_m, mcp->sum_tot, mcp->k_i, mcp->k_i_in);
//}
//
//double removal_modularity_loss(dynamic_graph *dwg, community_developer *cd, int node_index, int k_i_in) {
//	// I assume the removal loss is equal to the gain that we would get by
//	// putting the node back in its former community.
//
//	modularity_computing_package mcp;
//
//	// TODO
//	get_modularity_computing_package(&mcp, cd, node_index, *(cd->vertex_community+node_index), k_i_in);
//
//	mcp.sum_tot -= k_i_in;
//	mcp.sum_in = mcp.sum_in - 2 * k_i_in - (dwg->edges + node_index)->self_loop;
//
//	return modularity_delta(&mcp);
//}
//
//double removal_modularity_loss_weighted(dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int k_i_in) {
//	// I assume the removal loss is equal to the gain that we would get by
//	// putting the node back in its former community.
//
//	modularity_computing_package mcp;
//
//	// TODO
//	get_modularity_computing_package(&mcp, cd, node_index, *(cd->vertex_community+node_index), k_i_in);
//
//	mcp.sum_tot -= k_i_in;
//	mcp.sum_in = mcp.sum_in - 2 * k_i_in - (dwg->edges + node_index)->self_loop;
//
//	return modularity_delta(&mcp);
//}
//
//
//
////sorted_linked_list * get_neighbor_communities_list(sorted_linked_list *sll, dynamic_graph *dg, community_developer *cd, int node_index) {
////	dynamic_weighted_edge_array *neighbor_nodes;
////	int i;
////
////	if(!dg || !sll) {
////		printf("Invalid input in computation of neighbor communities!");
////		return 0;
////	}
////
////	if(node_index < 0 || node_index >= dg->size) {
////		printf("Invalid input in computation of neighbor communities: node out of range!");
////		return 0;
////	}
////
////	sorted_linked_list_free(sll);
////
////	neighbor_nodes = dg->edges + node_index;
////
////	//	Self loop not included in k_i_in of current community
////
////	//	if(neighbor_nodes->self_loop)
////	//		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community+node_index), neighbor_nodes->self_loop))) {
////	//			printf("Cannot insert node in sorted linked list!");
////	//
////	//			sorted_linked_list_free(sll);
////	//			free(sll);
////	//
////	//			return NULL;
////	//		}
////
////	for(i = 0; i < neighbor_nodes->count; i++) {
////		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), 1))) {
////			printf("Cannot insert node in sorted linked list!");
////
////			sorted_linked_list_free(sll);
////			free(sll);
////
////			return NULL;
////		}
////	}
////
////	return sll;
////}
//
//// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
//int get_neighbor_communities_list(sorted_linked_list *sll, dynamic_graph *dg, community_developer *cd, int node_index, int *current_community_k_i_in) {
//	dynamic_edge_array *neighbor_nodes;
//	int i;
//
//	int current_community;
//
//	*current_community_k_i_in = 0;
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
//	current_community = *(cd->vertex_community + node_index);
//
//	sorted_linked_list_free(sll);
//
//	neighbor_nodes = dg->edges + node_index;
//
////	Self loop not included in k_i_in of current community
//
////	if(neighbor_nodes->self_loop)
////		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community+node_index), neighbor_nodes->self_loop))) {
////			printf("Cannot insert node in sorted linked list!");
////
////			sorted_linked_list_free(sll);
////			free(sll);
////
////			return NULL;
////		}
//
//	for(i = 0; i < neighbor_nodes->count; i++) {
//		if(*(cd->vertex_community + (neighbor_nodes->addr + i)->dest) == current_community)
//			// Link internal to current node community
//			*current_community_k_i_in += 1;
//		else if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), 1))) {
//			printf("Cannot insert node in sorted linked list!");
//
//			sorted_linked_list_free(sll);
//			free(sll);
//
//			return 0;
//		}
//	}
//
//	return 1;
//}
//
//// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
//int get_neighbor_communities_list_weighted(sorted_linked_list *sll, dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int *current_community_k_i_in) {
//	dynamic_weighted_edge_array *neighbor_nodes;
//	int i;
//
//	int current_community;
//	*current_community_k_i_in = 0;
//
//	if(!dwg || !sll) {
//		printf("Invalid input in computation of neighbor communities!");
//		return 0;
//	}
//
//	if(node_index < 0 || node_index >= dwg->size) {
//		printf("Invalid input in computation of neighbor communities: node out of range!");
//		return 0;
//	}
//
//	current_community = *(cd->vertex_community + node_index);
//
//	sorted_linked_list_free(sll);
//
//	neighbor_nodes = dwg->edges + node_index;
//
////	Self loop not included in k_i_in of current community
//
////	if(neighbor_nodes->self_loop)
////		if(!(sorted_linked_list_insert(sll, *(cd->vertex_community+node_index), neighbor_nodes->self_loop))) {
////			printf("Cannot insert node in sorted linked list!");
////
////			sorted_linked_list_free(sll);
////			free(sll);
////
////			return NULL;
////		}
//
//	for(i = 0; i < neighbor_nodes->count; i++) {
//		if(*(cd->vertex_community + (neighbor_nodes->addr + i)->dest) == current_community)
//			// Link internal to current node community
//			*current_community_k_i_in += (neighbor_nodes->addr + i)->weight;
//		else if(!(sorted_linked_list_insert(sll, *(cd->vertex_community + (neighbor_nodes->addr + i)->dest), (neighbor_nodes->addr + i)->weight))) {
//			printf("Cannot insert node in sorted linked list!");
//
//			sorted_linked_list_free(sll);
//			free(sll);
//
//			return 0;
//		}
//	}
//
//	return 1;
//}
//
//// Copy weighted
////double compute_modularity(dynamic_graph *dg, community_developer *cd) {
////	double result;
////	int src,j;
////	int dest;
////
////	if(!dg || !cd){
////		printf("Invalid input: cannot compute modularity!");
////		return -2;
////	}
////
////	for(src=0;src<dg->size;src++)
////		for(j=0;j<(dg->edges+src)->count;j++) {
////			dest = ((dg->edges+src)->addr+j)->dest;
////			result += ((double) 1 - (double) *(cd->incoming_weight_node+src) * *(cd->incoming_weight_node+dest) / cd->double_m) * same(*(cd->vertex_community+src),*(cd->vertex_community+dest));
////		}
////
////	result /= cd->double_m;
////
////	return result;
////}
//
//inline double compute_modularity_edge_weighted(dynamic_weighted_graph *dwg, int edge_weight, int src_incoming_weight, int dest_incoming_weight, int double_m, int src_community, int dest_community) {
//	return ((double) edge_weight - (double) src_incoming_weight * dest_incoming_weight / double_m) * same(src_community,dest_community);
//}
//
//double compute_modularity_community_vector_weighted(dynamic_weighted_graph *dwg, int *community_vector) {
//	// TODO parallelize
//
//	int *incoming_weight;
//	int i,j;
//	int src, dest, src_incoming_weight, dest_incoming_weight, src_community, dest_community, src_self_loop, edge_weight;
//	int double_m;
//	double result;
//
//	if(!dwg || !community_vector) {
//		printf("Invalid input for modularity computation!\n");
//
//		return ILLEGAL_MODULARITY_VALUE;
//	}
//
//	if(!(incoming_weight = (int*) malloc(dwg->size * sizeof(int)))) {
//		printf("Couldn't allocate memory needed for modularity computation!\n");
//
//		return ILLEGAL_MODULARITY_VALUE;
//	}
//
//	for(i = 0; i < dwg->size; i++)
//		*(incoming_weight + i) = dynamic_weighted_graph_node_degree(dwg, i);
//
//	double_m = dynamic_weighted_graph_double_m(dwg);
//
//	result = 0;
//
//	for(src=0;src<dwg->size;src++) {
//		src_incoming_weight = *(incoming_weight + src);
//		src_community = *(community_vector + src);
//		src_self_loop = (dwg->edges + src)->self_loop;
//
//		for(j=0;j<(dwg->edges+src)->count;j++) {
//			dest = ((dwg->edges+src)->addr+j)->dest;
//			edge_weight = ((dwg->edges+src)->addr+j)->weight;
//			dest_incoming_weight = *(incoming_weight + dest);
//			dest_community = *(community_vector + dest);
//
////			result += ((double) edge_weight - (double) src_incoming_weight * dest_incoming_weight / cd->double_m) * same(src_community,dest_community);
//			result += compute_modularity_edge_weighted(dwg, edge_weight, src_incoming_weight, dest_incoming_weight, double_m, src_community, dest_community);
//		}
//
////		result += ((double) src_self_loop - (double) src_incoming_weight * src_incoming_weight / cd->double_m);
//		result += compute_modularity_edge_weighted(dwg, src_self_loop, src_incoming_weight, src_incoming_weight, double_m, src_community, src_community);
//	}
//
//	result /= double_m;
//
//	free(incoming_weight);
//
//	return result;
//}
//
//double compute_modularity_weighted(dynamic_weighted_graph *dwg, community_developer *cd) {
//	double result;
//	int src,j;
//	int dest, edge_weight;
//
//	int src_incoming_weight;
//	int dest_incoming_weight;
//
//	int src_community, dest_community;
//	int src_self_loop;
//
//	if(!dwg || !cd){
//		printf("Invalid input: cannot compute modularity!");
//		return ILLEGAL_MODULARITY_VALUE;
//	}
//
////	for(src=0;src<dwg->size;src++) {
////		for(j=0;j<(dwg->edges+src)->count;j++) {
////			dest = ((dwg->edges+src)->addr+j)->dest;
////			weight = ((dwg->edges+src)->addr+j)->weight;
////			result += ((double) weight - (double) *(cd->incoming_weight_node+src) * *(cd->incoming_weight_node+dest) / cd->double_m) * same(*(cd->vertex_community+src),*(cd->vertex_community+dest));
////		}
////
////		result +=
////	}
//
//	result = 0;
//
//	for(src=0;src<dwg->size;src++) {
//		src_incoming_weight = *(cd->incoming_weight_node+src);
//		src_community = *(cd->vertex_community+src);
//		src_self_loop = (dwg->edges + src)->self_loop;
//
//		for(j=0;j<(dwg->edges+src)->count;j++) {
//			dest = ((dwg->edges+src)->addr+j)->dest;
//			edge_weight = ((dwg->edges+src)->addr+j)->weight;
//			dest_incoming_weight = *(cd->incoming_weight_node+dest);
//			dest_community = *(cd->vertex_community+dest);
//
////			result += ((double) edge_weight - (double) src_incoming_weight * dest_incoming_weight / cd->double_m) * same(src_community,dest_community);
//			result += compute_modularity_edge_weighted(dwg, edge_weight, src_incoming_weight, dest_incoming_weight, cd->double_m, src_community, dest_community);
//		}
//
////		result += ((double) src_self_loop - (double) src_incoming_weight * src_incoming_weight / cd->double_m);
//		result += compute_modularity_edge_weighted(dwg, src_self_loop, src_incoming_weight, src_incoming_weight, cd->double_m, src_community, src_community);
//	}
//
//	result /= cd->double_m;
//
//	return result;
//}
//
//inline void set_exchange_ranking(community_exchange *ce, int node, int dest, int k_i_in_src, int k_i_in_dest, double modularity_delta) {
//	ce->node = node;
//	ce->dest = dest;
//	ce->k_i_in_src = k_i_in_src;
//	ce->k_i_in_dest = k_i_in_dest;
//	ce->modularity_delta = modularity_delta;
//}
//
//
//
//int sequential_select_pairings(community_developer *cd, community_exchange *exchange_rankings_sorted, int exchange_rankings_number, short **selected, int *stop_scanning_position) {
//	int used_communities = 0;
//	int *used_community;
//	int *used_node;
//	int current = 0;
//
//	*stop_scanning_position = exchange_rankings_number;
//
//	if(*selected = (short*) malloc(exchange_rankings_number * sizeof(short))) {
//		if(used_community = (int*) malloc(cd->n * sizeof(int))) {
//			if(used_node = (int*) malloc(cd->n * sizeof(int))) {
//
//				// pragma omp
//				memset(used_community, 0 , cd->n * sizeof(int));
//				memset(used_node, 0 , cd->n * sizeof(int));
//
//				while(used_communities < cd->n && current < exchange_rankings_number) {
//					if(!(*(used_node + (exchange_rankings_sorted+current)->node)) &&
//							!(*(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node))) &&
//							!(*(used_community + (exchange_rankings_sorted+current)->dest))) {
//						// Source and destination communities weren't used yet by any exchange
//
//						// Exchange selected
//						*(*selected + current) = 1;
//						// Vertex has been used
//						*(used_node + (exchange_rankings_sorted+current)->node) = 1;
//						// Source community used
//						*(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node)) = 1;
//						// Destination community used
//						*(used_community + (exchange_rankings_sorted+current)->dest) = 1;
//
//						used_communities += 2;
//
//						// Last selected exchange updated. Stop scanning selected array at position current + 1.
//						*stop_scanning_position = current + 1;
//					} else {
//						// Source and destination communities were already used by any exchange
//
//						// Exchange selected
//						*(*selected + current) = 0;
//					}
//
//					current++;
//				}
//
//				free(used_community);
//				free(used_node);
//			} else {
//				printf("Cannot allocate used node array!");
//				free(used_community);
//				free(*selected);
//
//				return 0;
//			}
//		} else {
//			printf("Cannot allocate used community array!");
//			free(*selected);
//
//			return 0;
//		}
//	} else {
//		printf("Cannot allocate selected array!");
//
//		return 0;
//	}
//
//	return 1;
//}
//
//inline void apply_transfer_weighted(dynamic_weighted_graph *dwg, community_developer *cd, community_exchange *exchange) {
//	const int src =  *(cd->vertex_community + exchange->node);
//	const int self_loop =  (dwg->edges + exchange->node)->self_loop;
//
//	// Update source community sum_tot (incoming_weight_community)
//	*(cd->incoming_weight_community + src) -= *(cd->incoming_weight_node + exchange->node);
//	// Update source community sum_int (internal_weight_community)
//	*(cd->internal_weight_community + src) -= (2 * exchange->k_i_in_src + self_loop);
//	// Update destination community sum_tot (incoming_weight_community)
//	*(cd->incoming_weight_community + exchange->dest) += *(cd->incoming_weight_node + exchange->node);
//	// Update destination community sum_int (internal_weight_community)
//	*(cd->internal_weight_community + exchange->dest) += (2 * exchange->k_i_in_dest + self_loop);
//
//	*(cd->vertex_community + exchange->node) = exchange->dest;
//
//	printf("Moving vertex %d from community %d to community %d.\n", exchange->node, src, exchange->dest);
//}
//
//int community_exchange_compare (const void * a, const void * b)
//{
//	double result = ( ((community_exchange*)a)->modularity_delta - ((community_exchange*)b)->modularity_delta );
//
//	if(result > 0)
//		return -1;
//	else if (result == 0)
//		return 0;
//	else
//		return 1;
//}
//
//int lower_power_of_2(int n) {
//	int result;
//
//	if(n <= 0)
//		return -1;
//
//	result = 1;
//
//	while(result * 2 <= n) {
//		result *= 2;
//	}
//
//	return result;
//}
//
//// If threads = 1 output sorted wont be used
//void community_exchange_parallel_quick_sort_merge(community_exchange *exchange_rankings, int first_partition_start, int first_partition_end, int second_partition_start, int second_partition_end,community_exchange *output_sorted) {
//	int i = first_partition_start;
//	int j = second_partition_start;
//
//	int compare_result;
//	int copy_to = first_partition_start;
//
//	while(i < first_partition_end && j < second_partition_end) {
//		compare_result = -community_exchange_compare(exchange_rankings+i, exchange_rankings+j);
//
//		if(compare_result > 0) {
//			// First greater then second
//			*(output_sorted+copy_to) = *(exchange_rankings+i);
//
//			i++;
//		} else if (compare_result == 0) {
//			// First and second are equal
//			*(output_sorted+copy_to) = *(exchange_rankings+i);
//			copy_to++;
//			*(output_sorted+copy_to) = *(exchange_rankings+j);
//
//			i++;
//			j++;
//
//		} else {
//			// Second greater than first
//			*(output_sorted+copy_to) = *(exchange_rankings+j);
//
//			j++;
//		}
//
//		copy_to++;
//	}
//
//	// If first partition was not fully copied
//	for(;i < first_partition_end;i++) {
//		*(output_sorted+copy_to) = *(exchange_rankings+i);
//		copy_to++;
//	}
//
//	// If second partition was not fully copied
//	for(;j < second_partition_end;j++) {
//		*(output_sorted+copy_to) = *(exchange_rankings+j);
//		copy_to++;
//	}
//}
//
//int community_exchange_parallel_quick_sort_main(community_exchange *exchange_rankings, int total_exchanges, int number_of_threads, community_exchange **output_sorted) {
//	int i;
//	int partitions_number = number_of_threads;
//	int partition_size;
//	int base_partition_size;
//
//	int first_partition_start;
//	int first_partition_end;
//	int second_partition_start;
//	int second_partition_end;
//
//	int num_threads;
//
//	int level;
//
//	*output_sorted = NULL;
//
//	// TODO Manage threads number properly
//
//	partitions_number = lower_power_of_2(number_of_threads);
//
//	if(partitions_number < 1) {
//		printf("Partitions number smaller than zero: can't sort.\n");
//
//		return 0;
//	}
//
//	base_partition_size = total_exchanges / partitions_number;
//
//	// Divide the input array in partition and qsort them in parallel
////	#pragma omp parallel for scheduling(static, 1) num_treads(partitions_number)
//	for(i = 0; i < partitions_number; i++)
//	{
//
//		if(i == (number_of_threads - 1) && total_exchanges > base_partition_size * partitions_number)
//			partition_size = total_exchanges - base_partition_size * partitions_number;
//		else
//			partition_size = base_partition_size;
//
//		qsort(exchange_rankings + i * partition_size, partition_size, sizeof(community_exchange), community_exchange_compare);
//	}
//
//	if(partitions_number > 1) {
//
//		if(!(*output_sorted = (community_exchange *) malloc(total_exchanges * sizeof(community_exchange)))) {
//			printf("Couldn't allocate space needed to merge partitions in sorting of community exchanges!");
//
//			return 0;
//		}
//
//		level = 0;
//
//		while(partitions_number != 1) {
//
//			// Merge subsections of the array in parallel
//			//		#pragma omp parallel for scheduling(static, 1) num_treads(partitions_number)
//			for(i = 0; i < partitions_number; i+=2) {
//
//				first_partition_start = i * base_partition_size;
//				second_partition_start = first_partition_end = (i + 1) * base_partition_size;
//
//				if(i == (partitions_number - 2) && total_exchanges > partitions_number * base_partition_size)
//					second_partition_end = total_exchanges - partitions_number * base_partition_size;
//				else
//					second_partition_end = (i + 2) * base_partition_size;
//
//				// Choose the correct layer containing the correct subsections to sort
//				if(level % 2)
//					community_exchange_parallel_quick_sort_merge(*output_sorted, first_partition_start, first_partition_end, second_partition_start, second_partition_end, exchange_rankings);
//				else
//					community_exchange_parallel_quick_sort_merge(exchange_rankings, first_partition_start, first_partition_end, second_partition_start, second_partition_end, *output_sorted);
//
//			}
//
//			partitions_number /= 2;
//			base_partition_size *= 2;
//			level++;
//		}
//
//		if(level % 2 == 0) {
//			// Output is contained in exchange_rankings, output_sorted is no more needed
//
//			free(*output_sorted);
//			*output_sorted = NULL;
//
//		}
//	}
//
//	return 1;
//}
//
//typedef struct temporary_community_edge {
//	int self_loop;
//	sorted_linked_list sll;
//} temporary_community_edge;
//
//void temporary_community_edge_init(temporary_community_edge *tce){
//	tce->self_loop = 0;
//	sorted_linked_list_init(&(tce->sll));
//}
//
//void temporary_community_edge_free(temporary_community_edge *tce){
//	sorted_linked_list_free(&(tce->sll));
//}
//
//int output_translator_weighted(dynamic_weighted_graph *dwg, community_developer *cd, dynamic_weighted_graph **community_graph, int **community_vector) {
//
//	// TODO free output, just in case
//
//	int *renumber_community;
//	int i,j;
//
//	int current_node;
//
//	int community_of_current_node;
//
//	int total_communities = 0;
//	int community;
//
//	temporary_community_edge *tce;
//
//	dynamic_weighted_edge_array neighbors;
//
//	weighted_edge neighbor;
//
//	sorted_linked_list_elem *neighbor_community;
//
//	printf("\n\n------------- OUTPUT TRANSLATION ------------\n\n");
//
//	printf("Input graph:\n\n");
//
//	dynamic_weighted_graph_print(*dwg);
//
//	printf("\n\nInput CD:\n\n");
//
//	community_developer_print(cd,0);
//
//	if(!(*community_graph = (dynamic_weighted_graph *) malloc (sizeof(dynamic_weighted_graph)))) {
//		printf("Cannot allocate output community graph!");
//
//		return 0;
//	}
//
//	if(!(*community_vector = (int *) malloc (cd->n * sizeof(int)))) {
//		printf("Cannot allocate non_empty_community community graph!");
//
//		free(*community_graph);
//		return 0;
//	}
//
//	if(!(renumber_community = (int *) malloc (cd->n * sizeof(int)))) {
//		printf("Cannot allocate renumber_community community graph!");
//
//		free(*community_graph);
//		free(*community_vector);
//		return 0;
//	}
//
////	#pragma omp parallel for scheduling(dynamic,10) shared(non_empty_community) private(i)
//	for(i = 0; i < cd->n ; i++)
//		*(renumber_community+i) = -1;
//
////	#pragma omp parallel for scheduling(dynamic,10) shared(non_empty_community) private(i, community)
//	for(i = 0; i < cd->n ; i++) {
//		community = *(cd->vertex_community + i);
//
//		if(*(renumber_community+community) != -1)
//			// Community is already re-numbered
//			*(*community_vector+i) = *(renumber_community+community);
//		else {
//			// Community isn't re-numbered yet
//			*(renumber_community+community) = total_communities;
//			*(*community_vector+i) = total_communities;
//
//			total_communities++;
//		}
//	}
//
//	free(renumber_community);
//
//	dynamic_weighted_graph_init(*community_graph, total_communities);
//
//	if(!(tce = (temporary_community_edge*) malloc(total_communities * sizeof(temporary_community_edge)))) {
//		printf("Cannot allocate temporary_community_edge community graph!");
//
//		free(*community_graph);
//		free(*community_vector);
//		return 0;
//	}
//
////	#pragma omp parallel for schedule(dynamic,10) private(i)
//	for(i = 0; i < total_communities; i++)
//		temporary_community_edge_init(tce+i);
//
//	// -------------------------------- SEQUENTIAL SECTION
//
//	for(current_node = 0; current_node < cd->n ; current_node++) {
//		neighbors = *(dwg->edges + current_node);
//		community_of_current_node = *(*community_vector+current_node);
//
//		for(j = 0; j < neighbors.count; j++){
//			neighbor = *(neighbors.addr+j);
//
//			if(*(*community_vector+neighbor.dest) == community_of_current_node)
//				// Community selfloop
//				(tce+community_of_current_node)->self_loop += neighbor.weight;
//			else
//				sorted_linked_list_insert(&((tce+community_of_current_node)->sll), *(*community_vector+neighbor.dest), neighbor.weight);
//		}
//	}
//
////	#pragma omp parallel for schedule(dynamic,5) private(i)
//	for(i = 0; i < total_communities; i++) {
//		((*community_graph)->edges + i)->self_loop = (tce+i)->self_loop;
//
//		neighbor_community = ((tce+i)->sll).head;
//		while(neighbor_community) {
//			dynamic_weighted_graph_insert(*community_graph, i, neighbor_community->community, neighbor_community->k_i_in);
//
//			neighbor_community = neighbor_community->next;
//		}
//	}
//
//	temporary_community_edge_free(tce);
//	free(tce);
//
//	return 1;
//}
//
//// Executes a phase of the algorithm, returns modularity gain
//double phase_weighted(dynamic_weighted_graph *dwg, double minimum_improvement, int num_threads, dynamic_weighted_graph **community_graph, int **community_vector) {
//	community_developer cd;
//
//	double initial_phase_modularity, final_phase_modularity;
//	double initial_iteration_modularity, final_iteration_modularity;
//
//	sorted_linked_list neighbors;
//	sorted_linked_list_elem *neighbor;
//	modularity_computing_package mcp;
//	double to_neighbor_modularity_delta;
//	int current_community_k_i_in;
//	double removal_loss;
//	double gain;
//	// Number of neighbor communities of a node worth considering for potential node transfer
//	int number_of_neighbor_communities;
//
//	community_exchange *node_exchanges_base_pointer;
//
//	int base,total_exchanges;
//
//	community_exchange *sorted_output_multi_thread;
//	// True if it is actually used (multi-threading)
//	int sorted_output_multi_thread_needs_free;
//
//	short *selected;
//	int stop_scanning_position;
//
//	int i,j;
//
//	int phase_iteration_counter = 0;
//
//	if(!dwg || minimum_improvement < 0 || minimum_improvement > 2) {
//		printf("Invalid phase parameters!");
//
//		return ILLEGAL_MODULARITY_VALUE;
//	}
//
//
//	community_developer_init_weighted(&cd, dwg);
//
//	community_developer_print(&cd,0);
//
//	initial_phase_modularity = compute_modularity_weighted(dwg, &cd);
//	final_iteration_modularity = initial_phase_modularity;
//
//	printf("\n\n---------------- Phase start -----------------\n\n");
//	printf("Initial phase modularity: %f\n", initial_phase_modularity);
//
//	do {
//		printf("\n\n---------------- Iteration #%d -----------------\n\n", phase_iteration_counter);
//
//		sorted_output_multi_thread_needs_free = 0;
//
//		initial_iteration_modularity = final_iteration_modularity;
//
//		printf("Initial iteration modularity: %f\n\n", initial_iteration_modularity);
//
//		// Do the parallel iterations over all nodes
//		// TODO put Parallel for - Everything else is good to go
//		for(i = 0; i < dwg->size; i++) {
//
//			if(i > 0)
//				node_exchanges_base_pointer = cd.exchange_ranking + *(cd.cumulative_edge_number + i - 1);
//			else
//				node_exchanges_base_pointer = cd.exchange_ranking;
//
//			// Node i, current edges: dwg->edges + i
//
//			number_of_neighbor_communities = 0;
//			sorted_linked_list_init(&neighbors);
//
//			// Compute neighbor communities and k_i_in
//			if(!get_neighbor_communities_list_weighted(&neighbors,dwg,&cd,i,&current_community_k_i_in)) {
//				printf("Could not compute neighbor communities!\n");
//
//				return ILLEGAL_MODULARITY_VALUE;
//			}
//
//			removal_loss = removal_modularity_loss_weighted(dwg, &cd, i, current_community_k_i_in);
//
//			neighbor = neighbors.head;
//			while(neighbor){
//				// Note that by the definition of get_neighbor_communities_list_weighted current community isn't in the list
//
//				// Compute modularity variation that would be achieved my moving node i from its current community to the neighbor community considered (neighbor->community)
//				get_modularity_computing_package(&mcp,&cd,i,neighbor->community,neighbor->k_i_in);
//				to_neighbor_modularity_delta = modularity_delta(&mcp);
//
//				// Check if the gain is worth to be considered
//				gain = to_neighbor_modularity_delta - removal_loss;
//				if(gain > MINIMUM_TRANSFER_GAIN) {
//					// Put computed potential transfer in its correct position for later community pairing selection
//
//					// TODO CRITICAL Shouldn't it be cd.cumulative_edge_number + i - 1 ?
//					set_exchange_ranking(node_exchanges_base_pointer + number_of_neighbor_communities,i,neighbor->community,current_community_k_i_in, neighbor->k_i_in,gain);
//
//					number_of_neighbor_communities += 1;
//				}
//
//				neighbor = neighbor->next;
//			}
//
//			sorted_linked_list_free(&neighbors);
//
//			*(cd.vertex_neighbor_communities + i) = number_of_neighbor_communities;
//		}
//
//		// -------------------------------- SEQUENTIAL SECTION
//
//		// Start copying in the first free position
//		base = 0;
//		total_exchanges = 0;
//		do {
//			total_exchanges += *(cd.vertex_neighbor_communities+base);
//			base++;
//		}while(base < dwg->size && *(cd.cumulative_edge_number+base-1) == total_exchanges);
//
//		for(i = base; i < dwg->size; i++)
//			for(j = 0; j < *(cd.vertex_neighbor_communities + i); j++) {
//				*(cd.exchange_ranking + total_exchanges) = *(cd.exchange_ranking + *(cd.cumulative_edge_number + i - 1) + j);
//
//				total_exchanges++;
//			}
//
//		// total_exchanges now represents the total number of active exchanges to be sorted
//
//		// -------------------------------- END OF SEQUENTIAL SECTION
//
//		// Do the parallel sorting of the community pairings array
//
//		if(!community_exchange_parallel_quick_sort_main(cd.exchange_ranking, total_exchanges, num_threads, &sorted_output_multi_thread)){
//			printf("Couldn't sort exchange pairings!");
//
//			return ILLEGAL_MODULARITY_VALUE;
//		}
//
//		if(sorted_output_multi_thread)
//			sorted_output_multi_thread_needs_free = 1;
//		else
//			sorted_output_multi_thread = cd.exchange_ranking;
//
//		// -------------------------------- SEQUENTIAL SECTION
//
//		// Do the sequential selection of the community pairings (iterate sequentially over the sorted ranking edges)
//
//		if(!sequential_select_pairings(&cd, sorted_output_multi_thread, total_exchanges, &selected, &stop_scanning_position)) {
//			printf("Couldn't select exchange pairings!");
//
//			return ILLEGAL_MODULARITY_VALUE;
//		}
//
//		// -------------------------------- END OF SEQUENTIAL SECTION
//
//		// Parallel updates for the communities
//
//		// TODO Parallel for
//		for(i = 0; i < total_exchanges; i++)
//			if(*(selected + i))
//				apply_transfer_weighted(dwg,&cd,sorted_output_multi_thread+i);
//
//		// Free memory used in the last computations
//		free(selected);
//		if(sorted_output_multi_thread_needs_free)
//			free(sorted_output_multi_thread);
//
//		// TODO End of substitute
//
//		final_iteration_modularity = compute_modularity_weighted(dwg, &cd);
//
//		printf("\n\nEnd of Iteration #%d - Result:\n\n", phase_iteration_counter);
//		community_developer_print(&cd,0);
//
//		phase_iteration_counter++;
//
//		printf("Final iteration modularity: %f. Modularity gain: %f\n", final_iteration_modularity, final_iteration_modularity - initial_iteration_modularity);
//
//	} while (final_iteration_modularity - initial_iteration_modularity > minimum_improvement);
//
//	output_translator_weighted(dwg, &cd, community_graph, community_vector);
//
//	community_developer_free(&cd);
//
//	final_phase_modularity = final_iteration_modularity;
//
//	// New communities graph and results should be passed
//	// TODO
//
//	printf("Final phase modularity: %f. Modularity gain: %f\n", final_phase_modularity, final_phase_modularity - initial_phase_modularity);
//
//	return final_phase_modularity - initial_phase_modularity;
//}
