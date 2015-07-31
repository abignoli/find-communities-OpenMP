/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

#include "execution-settings.h"

#define DEFAULT_MINIMUM_PHASE_IMPROVEMENT 0
#define DEFAULT_MINIMUM_ITERATION_IMPROVEMENT 0
#define DEFAULT_NUMBER_OF_THREADS 1
#define DEFAULT_SEQUENTIAL 0
#define DEFAULT_BENCHMARK_RUNS 0
#define DEFAULT_VERBOSE 0
#define DEFAULT_EXECUTION_SETTINGS_PARALLEL_PARTITIONS_HIGHER_POWER_OF_2 0
#define DEFAULT_FILE_FORMAT 0
#define DEFAULT_ALGORITHM_VERSION ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT
#define DEFINE_EXECUTION_SETTINGS_SORT_SELECT_CHUNKS_CHUNK_SIZE 0
#define DEFAULT_EXECUTION_SETTINGS_VERTEX_FOLLOWING 0

// Modify only if modifying also input parameters format!
#define MINIMUM_ARGUMENTS_NUMBER 2

void set_default(execution_settings *s);

void print_help(char *prog_name);

int parse_args(int argc, char *argv[], execution_settings *s);

void settings_print(execution_settings *settings);

#endif
