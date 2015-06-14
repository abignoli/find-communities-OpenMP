#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse-args.h"
#include "execution-settings.h"
#include "community-development.h"
#include "utilities.h"


void set_default(execution_settings *s) {
	s->input_file = NULL;
	s->graph_type = NOT_WEIGHTED;
	s->minimum_phase_improvement = DEFAULT_MINIMUM_PHASE_IMPROVEMENT;
	s->minimum_iteration_improvement = DEFAULT_MINIMUM_ITERATION_IMPROVEMENT;
	s->output_communities_file = NULL;
	s->output_graphs_file = NULL;
	s->number_of_threads = DEFAULT_NUMBER_OF_THREADS;
	s->sequential = DEFAULT_SEQUENTIAL;
	s->benchmark_runs = DEFAULT_BENCHMARK_RUNS;
	s->verbose = DEFAULT_VERBOSE;
	s->input_file_format = DEFAULT_FILE_FORMAT;

	s->execution_settings_parallel_partitions_higher_power_of_2 = DEFAULT_EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2;
}

void print_help(char *prog_name) {
	printf(PRINTING_UTILITY_STARS);

	printf("\n--- Usage:\n\n"
			"%s input-file [options]\n\n"
			"Available options:\n\n"
			"\t-h             Shows help\n"
			"\t                 * Ignores the rest of the input while doing so.\n"
			"\t-f number      Use file format identified by number.\n"
			"\t                 * Default is zero\n"
			"\t                 * Complete list of available file format options\n"
			"\t                   can be found below\n"
			"\t-t             Number of threads to use during parallel execution.\n"
			"\t                 * Must be greater than zero, default 1\n"
			"\t-s             Execute the sequential version of the algorithm\n"
			"\t               instead of the parallel one.\n"
			"\t                 * Given number of threads is ignored\n"
			"\t-v             Enable verbose logging.\n"
			"\t-p number      Stop phase analysis when phase improvement is smaller\n"
			"\t               than number.\n"
			"\t                 * Number must be between 0.0 and 1.0\n"
			"\t-i number      Stop phase internal iterations when iteration\n"
			"\t               improvement is smaller than number.\n"
			"\t                 * Number must be between 0.0 and 1.0\n"
			"\t-e number      Enable execution option identified by number.\n"
			"\t                 * Complete list of available execution options\n"
			"\t                   can be found below\n"
			"\t-c file        Save communities obtained in each phase in file.\n"
			"\t-g file        Save graphs obtained in each phase in file.\n"
			"\t-b number      Turn benchmarking on.\n"
			"\t                 * Perform the given number of benchmark runs\n"
			"\t                 * Disable file outputs and screen logging\n"
			"\t                 * Disable verbose logging\n"
			"\nAvailable execution options:\n\n"
			"\t-e %d           Use a number of partitions equal to the smallest\n"
			"\t                power of two greater or equal to the number of threads\n"
			"\t                during parallel sorting.\n"
			"\t                 * By default a number of partitions equal to the biggest\n"
			"\t                   power of two smaller or equal to the number of threads\n"
			"\t               	  is used\n"
			"\t                 * Applies only to parallel algorithm\n"
			"\nAvailable file format options:\n\n"
			"\t%d               Edge list, not weighted.\n"
			"\t                 * i.e. each line is of the form:\n"
			"\t                   source-node destination-node\n"
			"\t%d               Edge list, weighted.\n"
			"\t                 * i.e. each line is of the form:\n"
			"\t                   source-node destination-node edge-weight\n"
			"\t%d               Metis format.\n"
			"\nIndexes must be non negative, and weights should be greater than zero.\n"
			"\nOutput is undefined if any of the above conditions is not met.\n",
			prog_name,
			EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2_IDENTIFIER,
			FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED, FILE_FORMAT_EDGE_LIST_WEIGHTED, FILE_FORMAT_METIS);

	printf(PRINTING_UTILITY_STARS);
}

int parse_args(int argc, char *argv[], execution_settings *s){
	int valid = 1;
	int i, execution_option, file_format;

	set_default(s);

	if(argc < 2) {
		printf("Wrong number of arguments!\n");

		valid = 0;
	}

	if(valid && strcmp(argv[1],"-h") == 0) {
		valid = 0;
	}

	if(valid &&argc < MINIMUM_ARGUMENTS_NUMBER) {
		printf("Wrong number of arguments!\n");

		valid = 0;
	}

	if(valid) {

		s->input_file = argv[1];

		for(i=MINIMUM_ARGUMENTS_NUMBER; valid && i < argc; i++) {
			if(argv[i][0] == '-') {
				switch(argv[i][1]){

				case 'h':
					valid = 0;
					break;

				case 'f':
					if(i + 1 < argc) {
						i++;
						file_format = atoi(argv[i]);

						switch(file_format) {
						case FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED:
							s->input_file_format = FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED;
							break;
						case FILE_FORMAT_EDGE_LIST_WEIGHTED:
							s->input_file_format = FILE_FORMAT_EDGE_LIST_WEIGHTED;
							break;
						case FILE_FORMAT_METIS:
							s->input_file_format = FILE_FORMAT_METIS;
							break;

						default:
							printf("Invalid file format identifier '%s'!", argv[i]);
							valid = 0;
						}
					} else {
						printf("Expected file format identifier after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 't':
					if(i + 1 < argc) {
						i++;
						s->number_of_threads = atoi(argv[i]);

						if(s->number_of_threads <= 0) {
							printf("Invalid number of threads: '%s'", argv[i]);

							valid = 0;
						}
					} else {
						printf("Expected number of threads after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 's':
					s->sequential = 1;
					break;

				case 'v':
					s->verbose = 1;
					break;

				case 'p':
					if(i + 1 < argc) {
						i++;
						s->minimum_phase_improvement = atof(argv[i]);

						if(!valid_minimum_improvement(s->minimum_phase_improvement)) {
							printf("Invalid minimum phase improvement: '%s'", argv[i]);

							valid = 0;
						}
					} else {
						printf("Expected minimum phase improvement after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 'i':
					if(i + 1 < argc) {
						i++;
						s->minimum_iteration_improvement = atof(argv[i]);

						if(!valid_minimum_improvement(s->minimum_iteration_improvement)) {
							printf("Invalid minimum iteration improvement: '%s'", argv[i]);

							valid = 0;
						}
					} else {
						printf("Expected minimum iteration improvement after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 'b':
					if(i + 1 < argc) {
						i++;
						s->benchmark_runs = atoi(argv[i]);

						if(s->benchmark_runs <= 0) {
							printf("Invalid number of benchmark runs: '%s'", argv[i]);

							valid = 0;
						}
					} else {
						printf("Expected number of benchmark runs after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 'e':
					if(i + 1 < argc) {
						i++;
						execution_option = atoi(argv[i]);

						switch(execution_option) {
						case EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2_IDENTIFIER:
							s->execution_settings_parallel_partitions_higher_power_of_2 = 1;
							break;

						default:
							printf("Invalid execution option identifier '%s'!", argv[i]);
							valid = 0;
						}
					} else {
						printf("Expected execution option identifier after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 'c':
					if(i + 1 < argc) {
						i++;
						s->output_communities_file = argv[i];
					} else {
						printf("Expected output communities file name after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;

				case 'g':
					if(i + 1 < argc) {
						i++;
						s->output_graphs_file = argv[i];
					} else {
						printf("Expected output graphs file name after '%s'!\n", argv[i]);
						valid = 0;
					}
					break;
				default:
					printf("Invalid option '%s'!\n", argv[i]);
					valid = 0;
				}
			} else {
				printf("Invalid input: %s\n", argv[i]);

				valid = 0;
			}
		}
	}

	if(valid && s->benchmark_runs) {
		s->output_communities_file = s->output_graphs_file = NULL;
		s->verbose = 0;
	}


	if(!valid)
		print_help(argv[0]);

	return valid;
}

void settings_print(execution_settings *settings) {
	int some_execution_option_is_enabled = 0;

	printf(PRINTING_UTILITY_STARS);

	printf("Settings:\n\n");

	printf("\tInput file:                       %s\n", settings->input_file);
	printf("\tFile format:                      %s\n", file_format_name(settings->input_file_format));
	printf("\tMinimum phase improvement:        %f\n", settings->minimum_phase_improvement);
	printf("\tMinimum iteration improvement:    %f\n", settings->minimum_iteration_improvement);
	printf("\tAlgorithm version:                %s\n", (settings->sequential ? "Sequential" : "Parallel"));
	if(!settings->sequential)
		printf("\tNumber of threads:                %d\n", settings->number_of_threads);
	printf("\tVerbose logger:                   %s\n", (settings->verbose ? "Active" : "Inactive"));
	printf("\tBenchmarking:                     %s\n", (settings->benchmark_runs ? "Active" : "Inactive"));
	if(settings->benchmark_runs)
		printf("\tBenchmark runs:                   %d\n", settings->benchmark_runs);

	printf("\n");

	if(settings->output_communities_file)
		printf("\tSaving communities in:            %s\n", settings->output_communities_file);
	if(settings->output_graphs_file)
		printf("\tSaving community graphs in:       %s\n", settings->output_graphs_file);

	printf("\n");

	printf("\tEnabled execution options:        ");
	if(settings->execution_settings_parallel_partitions_higher_power_of_2) {
		some_execution_option_is_enabled = 1;
		printf("%d ", EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2_IDENTIFIER);
	}
	if(!some_execution_option_is_enabled)
		printf("None");

	printf("\n");
}
