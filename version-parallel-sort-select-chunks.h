#ifndef PARALLEL_SORT_SELECT_CHUNKS_H
#define PARALLEL_SORT_SELECT_CHUNKS_H

#define PARTITIONS_PER_THREAD_MULTIPLIER 1
#define MINIMUM_PARTITION_SIZE 500

#define NODE_ITERATION_CHUNK_SIZE 50
#define EXCHANGE_CHUNK_SIZE 60

#define DEFAULT_CHUNK_SIZE 2000

int phase_parallel_sort_select_chunks_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing);

#endif









