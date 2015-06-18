#include "utilities.h"
#include "execution-settings.h"
#include "execution-briefing.h"
#include "dynamic-weighted-graph.h"
#include "dynamic-graph.h"
#include "community-development.h"
#include "community-computation-weighted.h"
#include "community-computation-commons.h"
#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>
#include "version-parallel-naive-partitioning.h"
#include "community-computation-weighted-sequential.h"
#include "vertex-following.h"

int find_communities(dynamic_graph *dg ,dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, algorithm_execution_briefing *briefing) {
	int phase_counter;

	dynamic_weighted_graph *phase_output_community_graph;
	dynamic_weighted_graph *input_graph = dwg;

	phase_execution_briefing phase_briefing;

	double initial_phase_modularity, final_phase_modularity;

	FILE *output_communities_file;
	FILE *output_graphs_file;

	// For timing
	clock_t begin, end;
	double global_begin_wtime, global_end_wtime;
	double begin_wtime, end_wtime;
	double begin_pre_compute_wtime, end_pre_compute_wtime;
	double clock_time, wtime_omp;
	double global_wtime_omp;

	double minimum_phase_improvement = settings->minimum_phase_improvement;
	double minimum_iteration_improvement = settings->minimum_iteration_improvement;

	char *output_communities_filename = settings->output_communities_file;
	char *output_graphs_filename = settings->output_graphs_file;

	if(settings->graph_type == NOT_WEIGHTED) {
		printf("Executors for not weighted input graphs aren't implemented yet!");
		briefing->execution_successful = 0;

		return 0;
	}

	if(algorithm_version_parallel(settings->algorithm_version))
		omp_set_num_threads(settings->number_of_threads);
	else
		omp_set_num_threads(1);

	global_begin_wtime = omp_get_wtime();

	output_graphs_file = output_communities_file = NULL;

	if(!dwg || !valid_minimum_improvement(minimum_phase_improvement) || !valid_minimum_improvement(minimum_iteration_improvement)) {
		printf("Invalid algorithm parameters!");
		briefing->execution_successful = 0;

		return 0;
	}

	if(!settings->benchmark_runs) {
		// File IO is disabled during benchmark runs
		if(output_communities_filename) {
			output_communities_file = fopen (output_communities_filename, "w+");

			if(!output_communities_file) {
				printf("Could not open output communities file: %s", output_communities_filename);
				briefing->execution_successful = 0;

				return 0;
			}
		}

		if(output_graphs_filename) {
			output_graphs_file = fopen (output_graphs_filename, "w+");

			if(!output_graphs_file) {
				printf("Could not open output graphs file: %s", output_graphs_filename);
				briefing->execution_successful = 0;

				fclose(output_communities_file);
				return 0;
			}
		}
	}

	*community_vector = NULL;
	phase_output_community_graph = NULL;

	phase_counter = 0;

	if(!settings->benchmark_runs) {
		printf(PRINTING_UTILITY_EQUALS);
		printf(PRINTING_UTILITY_INDENT_TITLE);
		printf("Starting algorithm version: %s", algorithm_version_name(settings->algorithm_version));
	}

	clock_time = 0;
	wtime_omp = 0;
	briefing->precompute_time = 0;

	// Precomputations

	if(settings->execution_settings_vertex_following) {
		if(!settings->benchmark_runs) {
			printf(PRINTING_UTILITY_DASHES);
			printf("\nStarting Vertex Following heuristic\n\n");
		}

		begin_pre_compute_wtime = omp_get_wtime();

		if(!pre_compute_vertex_following(dwg,settings,&phase_output_community_graph, community_vector, &phase_briefing)) {
			printf("Could not perform vertex following heuristic!\n");

			return 0;
		}

		end_pre_compute_wtime = omp_get_wtime();

		briefing->precompute_time += end_pre_compute_wtime - begin_pre_compute_wtime;

		if(output_communities_file && !output_save_communities(output_communities_file, *community_vector, dwg->size)) {
			printf("Couldn't save communities output of phase #%d!\n",phase_counter);
			briefing->execution_successful = 0;

			return 0;
		}

		if(output_graphs_file && !output_save_community_graph(output_graphs_file, phase_output_community_graph, phase_counter)) {
			printf("Couldn't save graph output of phase #%d!\n",phase_counter);
			briefing->execution_successful = 0;

			return 0;
		}

		if(!settings->benchmark_runs) {
			printf("End of Vertex Following heuristic.\n Elapsed time: %f\n\n", end_pre_compute_wtime - begin_pre_compute_wtime);
		}

		dwg = phase_output_community_graph;
	}








	// TODO Insert optimized non weighted graph first phase here

	final_phase_modularity = compute_modularity_init_weighted_reference_implementation_method(dwg);

	// From this point onwards the input graph is considered weighted. If input graph is not weighted an additional special phase should be executed before this point
	do {
		free(*community_vector);

		initial_phase_modularity = final_phase_modularity;

		if(!settings->benchmark_runs) {
			printf(PRINTING_UTILITY_DASHES);
			printf("\n\nPHASE #%d:\n\nGraph size: %d\nInitial phase modularity: %f\n",phase_counter, dwg->size, final_phase_modularity);
		}

		if(settings->verbose) {
			printf("\nInitial graph:\n");
			dynamic_weighted_graph_print(*dwg);
		}

		if(settings->algorithm_version == ALGORITHM_VERSION_PARALLEL_2_NAIVE_PARTITION) {
			if(phase_counter == 0)
				// First phase, run partitioning
				settings->phase_executor_weighted = phase_parallel_naive_partitioning_weighted;
			else
				// Next phases, run sequential
				settings->phase_executor_weighted = sequential_phase_weighted;
		}

		// Just for performance measurement
		begin = clock();
		begin_wtime = omp_get_wtime();

		if(!(settings->phase_executor_weighted(dwg,settings,&phase_output_community_graph, community_vector, &phase_briefing))) {
			printf("Bad phase #%d computation!\n",phase_counter);
			briefing->execution_successful = 0;

			return 0;
		}

		final_phase_modularity = phase_briefing.output_modularity;

		// Just for performance measurement
		end_wtime = omp_get_wtime( );
		end = clock();
		clock_time += (double)(end - begin) / CLOCKS_PER_SEC;
		wtime_omp += end_wtime - begin_wtime;

		if(output_communities_file && !output_save_communities(output_communities_file, *community_vector, dwg->size)) {
			printf("Couldn't save communities output of phase #%d!\n",phase_counter);
			briefing->execution_successful = 0;

			return 0;
		}

		if(output_graphs_file && !output_save_community_graph(output_graphs_file, phase_output_community_graph, phase_counter)) {
			printf("Couldn't save graph output of phase #%d!\n",phase_counter);
			briefing->execution_successful = 0;

			return 0;
		}

		if(!settings->benchmark_runs) {
			printf("\nEnd of Phase #%d\n\n", phase_counter);
			printf("\tInitial modularity:                 %f\n",initial_phase_modularity);
			printf("\tFinal modularity:                   %f\n", final_phase_modularity);
			printf("\tGain:                               %f\n", final_phase_modularity - initial_phase_modularity);
			printf("\tExecution time:                     %fs\n", end_wtime - begin_wtime);
			printf("\tExecution time over all threads:    %fs\n", (double)(end - begin) / CLOCKS_PER_SEC);
			printf("\tNumber of iterations:               %d\n", phase_briefing.number_of_iterations);
		}

		// Clean memory
		// Avoids freeing initial input graph
		if(dwg != input_graph)
			dynamic_weighted_graph_free(dwg);

		// Prepare for next phase
		phase_counter++;
		dwg = phase_output_community_graph;
	} while(final_phase_modularity - initial_phase_modularity > minimum_phase_improvement);

	*community_graph = phase_output_community_graph;

	if(!settings->benchmark_runs) {
		printf(PRINTING_UTILITY_DASHES);
		printf("\n\nEnd of computation\n\n");
		printf("\tAlgorithm version:                   %s\n", algorithm_version_name(settings->algorithm_version));
		printf("\tMaximum number of threads:           %d\n", omp_get_max_threads());
		printf("\tExecution time:                      %fs\n",wtime_omp);
		printf("\tExecution time over all threads:     %fs\n", clock_time);
		printf("\tPrecomputation time:                 %fs\n", briefing->precompute_time);
		printf("\tFinal modularity:                    %f\n", final_phase_modularity);
		printf("\tNumber of phases:                    %d\n", phase_counter);

		printf(PRINTING_UTILITY_EQUALS);
	}

	// Saving output to file

	if(output_communities_file)
		fclose(output_communities_file);

	if(output_graphs_file)
		fclose(output_graphs_file);

	global_end_wtime = omp_get_wtime();

	global_wtime_omp = global_end_wtime - global_begin_wtime;

	briefing->execution_successful = 1;
	briefing->execution_time = wtime_omp;
	briefing->number_of_phases = phase_counter;
	briefing->clock_execution_time = clock_time;
	briefing->output_modularity = final_phase_modularity;
	briefing->global_execution_time = global_wtime_omp;

	return 1;
}
