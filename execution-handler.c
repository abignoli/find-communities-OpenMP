#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "execution-settings.h"
#include "utilities.h"
#include "community-computation-weighted-sequential.h"
#include "community-computation-weighted.h"
#include "execution-briefing.h"
#include "algorithm-executor.h"
#include "version-parallel-sort-select-chunks.h"
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

void merge_briefings_average(execution_briefing *briefing, algorithm_execution_briefing *internal_briefing, int number_of_previous_runs) {
	briefing->clock_execution_time = merge_average(briefing->clock_execution_time, number_of_previous_runs, internal_briefing->clock_execution_time, 1);
	briefing->execution_time = merge_average(briefing->execution_time, number_of_previous_runs, internal_briefing->execution_time, 1);
	briefing->precompute_time = merge_average(briefing->precompute_time, number_of_previous_runs, internal_briefing->precompute_time, 1);

	// Might be useful if some randomized heuristics are implemented later
	briefing->output_modularity = merge_average(briefing->output_modularity, number_of_previous_runs, internal_briefing->output_modularity, 1);
}

int select_phase_executors(execution_settings *settings) {
	// TODO Select non weighted phase executors when implemented
	switch(settings->algorithm_version) {
	case ALGORITHM_VERSION_SEQUENTIAL_0:
		// TODO Set proper non weighted phase executor
		settings->phase_executor_weighted = sequential_phase_weighted;
		break;
	case ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT:
		// TODO Set proper non weighted phase executor
		settings->phase_executor_weighted = parallel_phase_weighted;
		break;
	case ALGORITHM_VERSION_PARALLEL_2_NAIVE_PARTITION:
		// Handled internally to find communities
		break;
	case ALGORITHM_VERSION_PARALLEL_1_SORT_SELECT_CHUNKS:
		settings->phase_executor_weighted = phase_parallel_sort_select_chunks_weighted;
		break;
	default:
		printf("select_phase_executors - Unknown algorithm version!\n");
		return 0;
	}

	return 1;
}

int execute_community_detection(dynamic_graph *input_dg, dynamic_weighted_graph *input_dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, execution_briefing *briefing) {
	algorithm_execution_briefing internal_briefing;

	clock_t global_begin, global_end;
	double global_clock_time;

	double global_begin_wtime, global_end_wtime;
	double global_wtime_omp;

	briefing->performed_runs = 0;

	printf(PRINTING_UTILITY_STARS);
	printf("Starting execution...");

	// Select proper phase executors, depending on selected algorithm version
	if(!select_phase_executors(settings)) {
		printf("Could not set phase executors!\n");

		return 0;
	}

	if(!algorithm_version_parallel(settings->algorithm_version)) {
		global_begin = clock();
	} else {
		global_begin_wtime = omp_get_wtime();
	}

	printf(PRINTING_UTILITY_STARS);
	printf(PRINTING_UTILITY_INDENT_TITLE);
	printf("Run #%d of the algorithm", briefing->performed_runs + 1);

	if(!find_communities(input_dg, input_dwg, settings, community_graph, community_vector, &internal_briefing)) {
		printf("Could not complete execution!\n");

		return 0;
	}

	briefing->execution_time = internal_briefing.execution_time;
	briefing->clock_execution_time = internal_briefing.clock_execution_time;
	briefing->minimum_execution_time = internal_briefing.execution_time;
	briefing->minimum_clock_execution_time = internal_briefing.clock_execution_time;
	briefing->output_modularity = internal_briefing.output_modularity;

	briefing->performed_runs++;

	for(; briefing->performed_runs < settings->benchmark_runs; briefing->performed_runs++) {
		// Benchmarking is active, perform multiple runs and get average values

		dynamic_weighted_graph_free(*community_graph);
		free(*community_graph);
		free(*community_vector);

		printf(PRINTING_UTILITY_STARS);
		printf(PRINTING_UTILITY_INDENT_TITLE);
		printf("Run #%d of the algorithm", briefing->performed_runs + 1);

		if(!find_communities(input_dg, input_dwg, settings, community_graph, community_vector, &internal_briefing)) {
			printf("Could not complete execution!");

			return 0;
		}

		merge_briefings_average(briefing,&internal_briefing,briefing->performed_runs);
		if(briefing->minimum_execution_time > internal_briefing.execution_time)
			briefing->minimum_execution_time = internal_briefing.execution_time;
		if(briefing->minimum_clock_execution_time > internal_briefing.clock_execution_time)
			briefing->minimum_clock_execution_time = internal_briefing.clock_execution_time;

	}

	if(!algorithm_version_parallel(settings->algorithm_version)) {
		global_end = clock();

		global_clock_time = (double)(global_end - global_begin) / CLOCKS_PER_SEC;

		briefing->global_execution_time = global_clock_time;
	} else {
		global_end_wtime = omp_get_wtime();

		global_wtime_omp = global_end_wtime - global_begin_wtime;

		briefing->global_execution_time = global_wtime_omp;
	}

	briefing->execution_successful = 1;

	if(settings->benchmark_runs)
		execution_briefing_print(briefing);

	return 1;
}




