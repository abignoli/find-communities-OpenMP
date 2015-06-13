#include "dynamic-weighted-graph.h"
#include "community-computation-weighted.h"
#include "community-development.h"
#include "community-computation-weighted-sequential.h"
#include "utilities.h"
#include <stdio.h>
#include <time.h>

#include <omp.h>

//#define TEST_MAIN

#define NUM_THREADS 4



int main(int argc, char * argv[]){
	dynamic_weighted_graph dwg;

	dynamic_weighted_graph *output_dwg, *sequential_output_dwg;
	int *community_vector, *sequential_community_vector;

	clock_t begin, end;
	double begin_wtime, end_wtime;
	double sequential_time;
	double parallel_sequential_time_clock, parallel_sequential_time_wtime;

	int i;
	int num_threads = NUM_THREADS;

//	int reference_solution_example[] = { 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 2, 3, 2, 3, 2, 2};

#ifdef _OPENMP
	/* Set the number of threads */
	omp_set_num_threads(num_threads);
#endif

//	if(argc != 2) {
//		printf("Wrong number of arguments!\n");
//
//		return -1;
//	}
//
//	dynamic_weighted_graph_parse_file(&dwg, argv[1]);

//	if(dynamic_weighted_graph_parse_file(&dwg, "example.txt")) {
//		dynamic_weighted_graph_print(dwg);
//
//		printf("Executing a phase.\n\n");
//
//		phase_weighted(&dwg,0.0,4,&output_dwg,&community_vector);
//		phase_weighted_sequential(&dwg,0.0,&sequential_output_dwg,&sequential_community_vector);
//
//		printf("\n\n------ CHECK RESULTS -------\n\n");
//
//		printf("Parallel version output:\n\n");
//		dynamic_weighted_graph_print(*output_dwg);
//
//		printf("\n\nCommunity vector:\n\n");
//		for(i = 0; i < dwg.size; i++)
//			printf("%d ", *(community_vector + i));
//
//		printf("\n\nRe-computed modularity for parallel version (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, community_vector));
//
//		printf("Sequential version output:\n\n");
//		dynamic_weighted_graph_print(*sequential_output_dwg);
//
//		printf("\n\nCommunity vector:\n\n");
//		for(i = 0; i < dwg.size; i++)
//			printf("%d ", *(sequential_community_vector + i));
//
//		printf("\n\nRe-computed modularity for sequential version (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, sequential_community_vector));
//
//		printf("\n\nReference solution modularity (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, reference_solution_example));
//	} else
//		printf("Could not read input graph!");

	if(dynamic_weighted_graph_parse_file(&dwg, "arxiv.txt")) {
//		dynamic_weighted_graph_print(dwg);

		printf("Executing a phase.\n\n");

		// Parallel version
		begin = clock();
		begin_wtime = omp_get_wtime( );
		if(parallel_phase_weighted(&dwg,0.0,&output_dwg,&community_vector) == ILLEGAL_MODULARITY_VALUE)
				printf("Parallel phase execution terminated with errors!\n");
		end_wtime = omp_get_wtime( );
		end = clock();
		parallel_sequential_time_clock = (double)(end - begin) / CLOCKS_PER_SEC;
		parallel_sequential_time_wtime = end_wtime - begin_wtime;

		printf("Printing input graph:\n");
		dynamic_weighted_graph_print(dwg);
		printf("Community graph after parallel phase 0:\n");
		dynamic_weighted_graph_print(*output_dwg);

//		printf("\n\nReference solution modularity (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, reference_solution_example));

		// Sequential version
		begin = clock();
		if(sequential_phase_weighted(&dwg,0.0,&sequential_output_dwg,&sequential_community_vector) == ILLEGAL_MODULARITY_VALUE)
			printf("Sequential phase execution terminated with errors!\n");

		end = clock();
		sequential_time =  (double)(end - begin) / CLOCKS_PER_SEC;

		printf("\n\n------ CHECK RESULTS -------\n\n");

//		printf("Parallel version output:\n\n");
//		dynamic_weighted_graph_print(*output_dwg);
//
//		printf("\n\nCommunity vector:\n\n");
//		for(i = 0; i < dwg.size; i++)
//			printf("%d ", *(community_vector + i));

		printf("Execution time of first phase for parallel version executed by %d threads: %f (wtime) - %f (clock)\n",num_threads, parallel_sequential_time_wtime ,parallel_sequential_time_clock);
		printf("Re-computed modularity for parallel version (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, community_vector));

//		printf("\n\nReference solution modularity (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, reference_solution_example));

//		printf("Sequential version output:\n\n");
//		dynamic_weighted_graph_print(*sequential_output_dwg);
//
//		printf("\n\nCommunity vector:\n\n");
//		for(i = 0; i < dwg.size; i++)
//			printf("%d ", *(sequential_community_vector + i));

		printf("Execution time of first phase for sequential version way: %f\n", sequential_time);
		printf("Re-computed modularity for sequential version (from vector): %f\n\n", compute_modularity_community_vector_weighted(&dwg, sequential_community_vector));

		printf("Executing the whole algorithm.\n\n");

		for(i = 1; i <= NUM_THREADS; i++) {

#ifdef _OPENMP
			/* Set the number of threads */
			omp_set_num_threads(i);
#endif

			// Parallel version
			begin = clock();
			begin_wtime = omp_get_wtime( );
			if(parallel_find_communities_weighted(&dwg,0.0,0.0,"parallel-output-communities.txt","parallel-output-graph.txt", &output_dwg,&community_vector) == ILLEGAL_MODULARITY_VALUE)
				printf("Parallel algorithm execution terminated with errors!\n");
			end_wtime = omp_get_wtime( );
			end = clock();
			parallel_sequential_time_clock = (double)(end - begin) / CLOCKS_PER_SEC;
			parallel_sequential_time_wtime = end_wtime - begin_wtime;

//			printf("\n\nExecution time for full run of parallel algorithm executed by %d threads: %f (wtime) - %f (clock)\n",i, parallel_sequential_time_wtime ,parallel_sequential_time_clock);
//			printf("Re-computed modularity for parallel version (from vector): %f\n\n", compute_modularity_community_vector_weighted(output_dwg, community_vector));
		}
		begin = clock();
		if(sequential_find_communities_weighted(&dwg,0.0,0.0,"sequential-output-communities.txt","sequential-output-graph.txt", &output_dwg,&sequential_community_vector) == ILLEGAL_MODULARITY_VALUE)
			printf("Sequential phase execution terminated with errors!\n");
		end = clock();
		sequential_time =  (double)(end - begin) / CLOCKS_PER_SEC;

		printf("Execution time for full run of sequential algorithm: %f\n", sequential_time);
		printf("Re-computed modularity for sequential version (from vector): %f\n\n", compute_modularity_community_vector_weighted(output_dwg, sequential_community_vector));

	} else
		printf("Could not read input graph!");

	return 0;
}
