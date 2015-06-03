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

	dynamic_weighted_graph_parse_file(&dwg, "/home/abub/data/programming/ide/eclipse-parallel-development/workspace/find-communities-openmp/Debug/test.txt");

	dynamic_weighted_graph_print(dwg);

	phase_weighted(&dwg,0,4,&output_dwg,&community_vector);

	dynamic_weighted_graph_print(output_dwg);

	return 0;
}
