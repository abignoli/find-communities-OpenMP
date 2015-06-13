#include "parse-args.h"
#include "execution-settings.h"
#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "input-handler.h"
#include "execution-handler.h"
#include "utilities.h"
#include "community-development.h"
#include <stdio.h>

int main(int argc, char * argv[]) {
	execution_settings settings;
	dynamic_graph input_dg;
	dynamic_weighted_graph input_dwg;

	dynamic_weighted_graph *output_dwg;
	int *output_communities;

	if(!parse_args(argc, argv, &settings))
		// Invalid input settings
		return -1;

	settings_print(&settings);

	printf("Parsing input graph...\n\n");

	if(!parse_input(&input_dg, &input_dwg, &settings))
		return -1;

	if(execute_community_detection(&input_dg, &input_dwg, &settings, &output_dwg, &output_communities) == ILLEGAL_MODULARITY_VALUE)
		printf("Community detection exited with errors!\n");

	free(output_communities);
	dynamic_weighted_graph_free(output_dwg);
	free(output_dwg);

//	if(settings->graph_type == NOT_WEIGHTED) {
//		printf(PRINTING_NOT_YET_IMPLEMENTED);
//		return;
//	}

	printf(PRINTING_UTILITY_STARS);
}

