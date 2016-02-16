/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef VERSION_PARALLEL_NAIVE_PARTITIONING_H
#define VERSION_PARALLEL_NAIVE_PARTITIONING_H

#define PARTITIONS_PER_THREAD_MULTIPLIER 1
#define MINIMUM_PARTITION_SIZE 500

int phase_parallel_naive_partitioning_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing);

#endif
