#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H

typedef struct execution_settings execution_settings;

#define DEFAULT_MINIMUM_PHASE_IMPROVEMENT 0
#define DEFAULT_MINIMUM_ITERATION_IMPROVEMENT 0

#define DEFAULT_NUMBER_OF_THREADS 1

#define DEFAULT_SEQUENTIAL 0

#define DEFAULT_BENCHMARK_RUNS 0

// Modify only if modifying also input parameters format!
#define MINIMUM_ARGUMENTS_NUMBER 3

void set_default(execution_settings *s);

void print_help(char *prog_name);

int parse_args(int argc, char *argv[], execution_settings *s);

void settings_print(execution_settings *settings);

#endif
