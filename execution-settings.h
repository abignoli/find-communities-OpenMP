#ifndef EXECUTION_SETTINGS_H
#define EXECUTION_SETTINGS_H

#define NOT_WEIGHTED 0
#define WEIGHTED 1

typedef struct execution_settings {
	char *input_file;
	int graph_type;
	double minimum_phase_improvement;
	double minimum_iteration_improvement;
	char *output_communities_file;
	char *output_graphs_file;
	int number_of_threads;
	// Tells that the program should be executed using the sequential implementation
	int sequential;
	int benchmark_runs;
} execution_settings;

#endif
