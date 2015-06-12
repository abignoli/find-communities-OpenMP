#include "parse-args.h"
#include "execution-settings.h"
#include "dynamic-weighted-graph.h"

#define NOT_WEIGHTED_GRAPH_OPTIMIZATION_NOT_YET_IMPLEMENTED

int main(int argc, char * argv[]) {
	execution_settings settings;
	dynamic_graph input_dg;
	dynamic_weighted_graph input_dwg;

	parse_args(argc, argv, &settings);

	settings_print(&settings);

	if(settings->graph_type == WEIGHTED) {
		dynamic_weighted_graph_parse_file(&input_dwg, settings->input_file);
	} else {
#ifdef NOT_WEIGHTED_GRAPH_OPTIMIZATION_NOT_YET_IMPLEMENTED
		dynamic_weighted_graph_parse_not_weighted_file(&input_dwg, settings->input_file, DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES);
		settings->graph_type = WEIGHTED;
#else
		printf(PRINTING_NOT_YET_IMPLEMENTED);
		return;
#endif
	}

	if(settings->graph_type == NOT_WEIGHTED) {
		printf(PRINTING_NOT_YET_IMPLEMENTED);
		return;
	}







}
