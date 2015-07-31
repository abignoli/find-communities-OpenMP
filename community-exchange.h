/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef COMMUNITY_EXCHANGE_H
#define COMMUNITY_EXCHANGE_H

typedef struct community_developer community_developer;
typedef struct execution_settings execution_settings;

typedef struct community_exchange {
	int node;
	int dest;
	int k_i_in_src;
	int k_i_in_dest;
	double modularity_delta;
} community_exchange;

inline void set_exchange_ranking(community_exchange *ce, int node, int dest, int k_i_in_src, int k_i_in_dest, double modularity_delta);

int sequential_select_pairings(community_developer *cd, community_exchange *exchange_rankings_sorted, int exchange_rankings_number, short **selected, int *stop_scanning_position);

int community_exchange_compare (const void * a, const void * b);

// If threads = 1 output sorted wont be used
void community_exchange_parallel_quick_sort_merge(community_exchange *exchange_rankings, int first_partition_start, int first_partition_end, int second_partition_start, int second_partition_end,community_exchange *output_sorted);

int community_exchange_parallel_quick_sort_main(community_exchange *exchange_rankings, int total_exchanges, execution_settings *settings, community_exchange **output_sorted);

#endif
