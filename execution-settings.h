#ifndef EXECUTION_SETTINGS_H
#define EXECUTION_SETTINGS_H

#define NOT_WEIGHTED 0
#define WEIGHTED 1

#define EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2_IDENTIFIER 0

#define FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED 0
#define FILE_FORMAT_EDGE_LIST_WEIGHTED 1
#define FILE_FORMAT_METIS 2

#define FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED_NAME "Edge list, not weighted"
#define FILE_FORMAT_EDGE_LIST_WEIGHTED_NAME "Edge list, weighted"
#define FILE_FORMAT_METIS_NAME "Metis"
#define FILE_FORMAT_INVALID_NAME "Invalid file format!"

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
	int sequential;
	int benchmark_runs;

	int execution_settings_parallel_partitions_higher_power_of_2;
} execution_settings;

char * file_format_name(int id);

#endif
