#include <stdio.h>
#include <stdlib.h>

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
}

void print_help(char *prog_name) {
	printf(PRINTING_UTILITY_STARS);

	printf("\n--- Usage:\n\n"
			"%s input-file output-communities-file [options]\n\n"
			"Available options:\n\n"
			"\t-h\t\tShows help (Ignores the rest of the input while doing so)\n"
			"\t-w\t\tInput graph is weighted (Only available option as of now). Default is non-weighted\n"
			"\t-t\t\tNumber of threads to use during parallel execution (Must be greater than zero, default 1)\n"
			"\t-s\t\tExecute the sequential version of the algorithm instead of the parallel one. (Given number of threads is ignored)\n"
			"\t-p number\tStop phase analysis when phase improvement is smaller than number. Number must be between 0.0 and 1.0\n"
			"\t-i number\tStop iteration (over all nodes) analysis when iteration improvement is smaller than number. Number must be between 0.0 and 1.0\n"
			"\t-o file\t\tSave graphs obtained in each phase in file\n\n"
			"Input file must have the format:\n\n"
			"source-node destination-node [edge-weight]\n\n"
			"Indexes must be non negative, and weights should be greater than zero. The output is undefined if these conditions are not met\n", prog_name);

	printf(PRINTING_UTILITY_STARS);
}

int parse_args(int argc, char *argv[], execution_settings *s){
	int valid = 1;
	int i;

	set_default(s);

	if(argc < MINIMUM_ARGUMENTS_NUMBER) {
		printf("Wrong number of arguments!\n");

		valid = 0;
	}

	if(valid) {
		s->input_file = argv[1];
		s->output_communities_file = argv[2];

		for(i=MINIMUM_ARGUMENTS_NUMBER; valid && i < argc; i++) {
			if(argv[i][0] == '-') {
				switch(argv[i][1]){

				case 'h':
					valid = 0;
					break;

				case 'w':
					s->graph_type = WEIGHTED;
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

				case 'o':
					if(i + 1 < argc) {
						i++;
						s->output_graphs_file = argv[i];
					} else {
						printf("Expected output file name after '%s'!\n", argv[i]);
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

	if(!valid)
		print_help(argv[0]);

	return valid;
}

void settings_print(execution_settings *settings) {
	printf(PRINTING_UTILITY_STARS);

	printf("Settings:\n\n");

	printf("\tInput file: %s\n", settings->input_file);
	printf("\tOutput file (communities): %s\n", settings->output_communities_file);
	printf("\tGraph type: %s\n", (settings->graph_type == WEIGHTED ? "Weighted" : "Not Weighted"));
	printf("\tMinimum phase improvement: %f\n", settings->minimum_phase_improvement);
	printf("\tMinimum iteration improvement: %f\n", settings->minimum_iteration_improvement);
	printf("\tAlgorithm version: %s\n", (settings->sequential ? "Sequential" : "Parallel"));
	if(!settings->sequential)
		printf("\tNumber of threads: %d\n", settings->number_of_threads);

	printf("\n");

	if(settings->output_graphs_file)
		printf("\tSaving community graphs in: %s\n", settings->output_graphs_file);
}
