#ifndef COMMUNITY_COMPUTATION_WEIGHTED_H
#define COMMUNITY_COMPUTATION_WEIGHTED_H

typedef struct community_developer community_developer;
typedef struct community_exchange community_exchange;
typedef struct dynamic_weighted_graph dynamic_weighted_graph;
typedef struct sorted_linked_list sorted_linked_list;

// Designed to be called at the start of each phase
int community_developer_init_weighted(community_developer *cd, dynamic_weighted_graph *dwg);

double removal_modularity_loss_weighted(dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int k_i_in);

// Note: Current community not included in sll. Internal links are considered and their sum is put into current_community_k_i_in. Self loops are ignored
int get_neighbor_communities_list_weighted(sorted_linked_list *sll, dynamic_weighted_graph *dwg, community_developer *cd, int node_index, int *current_community_k_i_in);

inline double compute_modularity_edge_weighted(dynamic_weighted_graph *dwg, int edge_weight, int src_incoming_weight, int dest_incoming_weight, int double_m, int src_community, int dest_community);

double compute_modularity_community_vector_weighted(dynamic_weighted_graph *dwg, int *community_vector);

double compute_modularity_weighted(dynamic_weighted_graph *dwg, community_developer *cd);

inline void apply_transfer_weighted(dynamic_weighted_graph *dwg, community_developer *cd, community_exchange *exchange);

int output_translator_weighted(dynamic_weighted_graph *dwg, community_developer *cd, dynamic_weighted_graph **community_graph, int **community_vector);

// Executes a phase of the algorithm, returns modularity gain
double phase_weighted(dynamic_weighted_graph *dwg, double minimum_improvement, dynamic_weighted_graph **community_graph, int **community_vector);

#endif
