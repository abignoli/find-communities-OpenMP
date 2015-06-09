#define NOT_WEIGHTED 0
#define WEIGHTED 1

typedef struct settings {
	char *input_file;
	int graph_type;
	double minimum_phase_improvement;
	double minimum_iteration_improvement;
	char *output_graphs_file;
	int number_of_threads;
} settings;

void set_default(settings *s);

void print_help(char *prog_name);

int parse_args(int argc, char *argv[], settings *s);
