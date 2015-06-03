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

typedef struct community_exchange {
	// TODO Decide whether to memoize the delta relative to community parameters

	int node;
	int dest;
	int k_i_in_src;
	int k_i_in_dest;
	float modularity_delta;
} community_exchange;

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

	// Summing cumulative degree of each node to partition pair ranking array among vertex (and threads)
	int* cumulative_edge_number;
	// Number of neighbor communities of each vertex. This should be used when doing the sorting
	int* vertex_neighbor_communities;
	// To be used to store modularity increments relative to node movement and then rank best pairings
	community_exchange* exchange_ranking;
} community_developer;
