#include "parse-args.h"
#include "execution-settings.h"
#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include <stdio.h>

int parse_input(dynamic_graph *dg, dynamic_weighted_graph *dwg, execution_settings *settings) {
	int valid = 1;

	if(settings->graph_type == WEIGHTED) {
		if(!dynamic_weighted_graph_parse_file(dwg, settings->input_file))
			valid = 0;
	} else {
		// TODO Use actual not weighted graph
		settings->graph_type = WEIGHTED;

		if(!dynamic_weighted_graph_parse_not_weighted_file(dwg, settings->input_file, DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES))
			valid = 0;
	}

	if(!valid){
		printf("Failed to read input file: %s\n", settings->input_file);
		return 0;
	}

	return 1;
}
