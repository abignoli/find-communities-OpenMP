#ifndef EXECUTION_SETTINGS_H
#define EXECUTION_SETTINGS_H

typedef struct dynamic_graph dynamic_graph;
typedef struct dynamic_weighted_graph dynamic_weighted_graph;
typedef struct execution_settings execution_settings;
typedef struct phase_execution_briefing phase_execution_briefing;

#define NOT_WEIGHTED 0
#define WEIGHTED 1

#define EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2_IDENTIFIER 0
#define EXECUTION_SETTINGS_SORT_SELECT_CHUNKS_CHUNK_SIZE_IDENTIFIER 1
#define EXECUTION_SETTINGS_VERTEX_FOLLOWING_IDENTIFIER 2

#define FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED 0
#define FILE_FORMAT_EDGE_LIST_WEIGHTED 1
#define FILE_FORMAT_METIS 2

#define FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED_NAME "Edge list, not weighted"
#define FILE_FORMAT_EDGE_LIST_WEIGHTED_NAME "Edge list, weighted"
#define FILE_FORMAT_METIS_NAME "Metis"
#define FILE_FORMAT_INVALID_NAME "Invalid file format!"

#define ALGORITHM_VERSION_SEQUENTIAL_0 0
#define ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT 1
#define ALGORITHM_VERSION_PARALLEL_2_NAIVE_PARTITION 2
#define ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT_CHUNKS 3

#define ALGORITHM_VERSION_SEQUENTIAL_0_NAME "Sequential"
#define ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT_NAME "Parallel (Sort & Select)"
#define ALGORITHM_VERSION_PARALLEL_2_NAIVE_PARTITION_NAME "Parallel (Naive partitioning on first phase)"
#define ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT_CHUNKS_NAME "Parallel (Sort & Select Chunk)"
#define ALGORITHM_VERSION_INVALID_NAME "Invalid algorithm version!"

typedef struct execution_settings {
	char *input_file;
	int input_file_format;
	int graph_type;
	double minimum_phase_improvement;
	double minimum_iteration_improvement;
	char *output_communities_file;
	char *output_graphs_file;
	int number_of_threads;
	int verbose;

	// Tells that the program should be executed using the sequential implementation
//	int sequential;
	int benchmark_runs;

	int execution_settings_parallel_partitions_higher_power_of_2;

	int execution_settings_sort_select_chunks_chunk_size;

	int execution_settings_vertex_following;

	int algorithm_version;
	// Phase executors, set by execution handler depending on chosen algorithm version. Could potentially be controlled at runtime
	int (*phase_executor_weighted)(dynamic_weighted_graph *, execution_settings *, dynamic_weighted_graph **, int **, phase_execution_briefing *);
	int (*phase_executor_not_weighted)(dynamic_graph *, execution_settings *, dynamic_weighted_graph **, int **, phase_execution_briefing *);

} execution_settings;

char * file_format_name(int id);

int algorithm_version_parallel(int id);

char * algorithm_version_name(int id);

#endif
