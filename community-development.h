#ifndef COMMUNITY_DEVELOPMENT_H
#define COMMUNITY_DEVELOPMENT_H

typedef struct community_exchange community_exchange;
typedef struct dynamic_weighted_graph dynamic_weighted_graph;
typedef struct sorted_linked_list sorted_linked_list;
typedef struct temporary_community_edge temporary_community_edge;

// Needs to be >= 0 otherwise the algorithm can't converge!
#define MINIMUM_TRANSFER_GAIN 0
#define ILLEGAL_MODULARITY_VALUE -2
#define MINIMUM_LEGAL_MODULARITY -0.5
#define MAXIMUM_LEGAL_MODULARITY 1.0

#define MINIMUM_LEGAL_IMPROVEMENT 0
#define MAXIMUM_LEGAL_IMPROVEMENT 2

typedef struct modularity_computing_package {
	// Sum of the weights of the links internal to the community
	int sum_in;
	// Sum of the weights of the links incident to node in the community, includes also internal links
	int sum_tot;
	// Sum of the weights of the links incident to the node
	int k_i;
	// Sum of the weights of the links from the node to the community
	int k_i_in;
	// Sum of the weights in the graph
	int double_m;
} modularity_computing_package;

typedef struct community_developer {
	// n is the number of vertices, which is also the number of communities considered in the phase
	int n;
	// Sum of the weights in the graph
	int double_m;

	int *vertex_community;

	// Sum of the weights of the links internal to each community
	int* internal_weight_community;
	// Sum of the weights of the links incident to node in each community, includes also internal links
	int* incoming_weight_community;
	// Sum of the weights of the links incident to each node
	int* incoming_weight_node;

	// Summing cumulative degree of each node to partition pair ranking array among vertexes (and threads)
	int* cumulative_edge_number;
	// Number of neighbor communities of each vertex. This should be used when doing the sorting
	int* vertex_neighbor_communities;
	// To be used to store modularity increments relative to node movement and then rank best pairings
	community_exchange* exchange_ranking;
} community_developer;

// Designed to be called at the start of each phase
int community_developer_init_weighted(community_developer *cd, dynamic_weighted_graph *dwg);

int valid_minimum_improvement(double mi);

void community_developer_free(community_developer *cd);

void community_developer_print(community_developer *cd, int total_exchanges);

inline void get_modularity_computing_package(modularity_computing_package *mcp,community_developer *cd, int node_index, int community_index, int k_i_in);

inline double modularity_delta_unpackaged(int sum_in, int double_m, int sum_tot, int k_i, int k_i_in);

inline double modularity_delta(modularity_computing_package *mcp);


#endif
