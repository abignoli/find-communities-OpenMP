#include "community-development.h"

#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "sorted-linked-list.h"
#include "utilities.h"
#include <stdlib.h>
#include <omp.h>
#include "community-exchange.h"
#include "silent-switch.h"
#include "temporary-community-edge.h"

#ifdef SILENT_SWITCH_ON_COMMUNITY_DEVELOPMENT
#define printf(...)
#endif

int valid_minimum_improvement(double mi) {
	return mi >= MINIMUM_LEGAL_IMPROVEMENT && mi <= MAXIMUM_LEGAL_IMPROVEMENT;
}

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

void community_developer_print(community_developer *cd, int total_exchanges) {
	int i;

	printf("\n\n---------------- PRINTING COMMUNITY DEVELOPER ----------------\n\n");

	printf("Number of nodes: %d\nTotal weight in graph: %d\n\n", cd->n, cd->double_m);

	for(i = 0; i < cd->n; i++)
		printf("-- Node %d:\n\nCommunity: %d\nNeighbor communities: %d\nIncoming node weight: %d\nCumulative edge number: %d\n\n ", i, *(cd->vertex_community + i), *(cd->vertex_neighbor_communities + i),*(cd->incoming_weight_node + i),*(cd->cumulative_edge_number + i));

	for(i = 0; i < cd->n; i++)
		printf("-- Community %d:\n\nIncoming weight: %d\nInternal weight: %d\n\n",i, *(cd->incoming_weight_community + i), *(cd->internal_weight_community + i));

	for(i = 0; i < total_exchanges; i++)
		printf("-- Potential exchange:\n\nNode to transfer: %d\nDestination community: %d\nSource community k_i_in: %d\nDestination community k_i_in: %d\nModularity delta: %f\n\n", (cd->exchange_ranking + i)->node, (cd->exchange_ranking + i)->dest, (cd->exchange_ranking + i)->k_i_in_src, (cd->exchange_ranking + i)->k_i_in_dest, (cd->exchange_ranking + i)->modularity_delta);

	printf("\n\n------------- END OF PRINTING COMMUNITY DEVELOPER -------------\n\n");

}

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

inline void get_modularity_computing_package(modularity_computing_package *mcp,community_developer *cd, int node_index, int community_index, int k_i_in) {
	mcp->k_i = *(cd->incoming_weight_node+node_index);
	mcp->k_i_in = k_i_in;
	mcp->sum_in = *(cd->internal_weight_community+community_index);
	mcp->sum_tot = *(cd->incoming_weight_community+community_index);
	mcp->double_m = cd->double_m;
}

inline double modularity_delta_unpackaged(int sum_in, int double_m, int sum_tot, int k_i, int k_i_in) {
	double res;
	double a = ((double) (sum_tot + k_i));

	res = ((double)(sum_in + 2 * k_i_in)) - a * a /double_m;

	a = ((double) sum_tot);

	res -= ((double) sum_in - a * a / double_m - ((double) k_i * k_i) / double_m);

	res /= double_m;

	return res;
}

inline double modularity_delta(modularity_computing_package *mcp) {
	return modularity_delta_unpackaged(mcp->sum_in, mcp->double_m, mcp->sum_tot, mcp->k_i, mcp->k_i_in);
}

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



// Copy weighted
//double compute_modularity(dynamic_graph *dg, community_developer *cd) {
//	double result;
//	int src,j;
//	int dest;
//
//	if(!dg || !cd){
//		printf("Invalid input: cannot compute modularity!");
//		return -2;
//	}
//
//	for(src=0;src<dg->size;src++)
//		for(j=0;j<(dg->edges+src)->count;j++) {
//			dest = ((dg->edges+src)->addr+j)->dest;
//			result += ((double) 1 - (double) *(cd->incoming_weight_node+src) * *(cd->incoming_weight_node+dest) / cd->double_m) * same(*(cd->vertex_community+src),*(cd->vertex_community+dest));
//		}
//
//	result /= cd->double_m;
//
//	return result;
//}

