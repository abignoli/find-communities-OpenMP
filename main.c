#include "dynamic-weighted-graph.h"
#include "community-development.h"

int main(int argc, char * argv[]){
	dynamic_weighted_graph dwg;

	dynamic_weighted_graph output_dwg;
	int *community_vector;

//	if(argc != 2) {
//		printf("Wrong number of arguments!\n");
//
//		return -1;
//	}
//
//	dynamic_weighted_graph_parse_file(&dwg, argv[1]);

	if(dynamic_weighted_graph_parse_file(&dwg, "test.txt")) {
		dynamic_weighted_graph_print(dwg);

		printf("Executing a phase.\n\n");

		phase_weighted(&dwg,0,4,&output_dwg,&community_vector);

		printf("End of phase. Output:\n\n");
		dynamic_weighted_graph_print(output_dwg);
	} else
		printf("Could not read input graph!");

	return 0;
}
