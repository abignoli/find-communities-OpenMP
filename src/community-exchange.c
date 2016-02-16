/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "community-exchange.h"

#include "community-development.h"
#include "silent-switch.h"
#include "execution-settings.h"

#include "printing_controller.h"

#include <stdio.h>
#include <omp.h>

//#ifdef SILENT_SWITCH_SORT_ON
//#define printf(...)
//#endif

#define MINIMUM_PARTITION_SIZE 100

inline void set_exchange_ranking(community_exchange *ce, int node, int dest, int k_i_in_src, int k_i_in_dest, double modularity_delta) {
	ce->node = node;
	ce->dest = dest;
	ce->k_i_in_src = k_i_in_src;
	ce->k_i_in_dest = k_i_in_dest;
	ce->modularity_delta = modularity_delta;
}

int sequential_select_pairings(community_developer *cd, community_exchange *exchange_rankings_sorted, int exchange_rankings_number, short **selected, int *stop_scanning_position) {
	int used_communities = 0;
	int *used_community;
	int *used_node;
	int current = 0;

	int node_has_been_used;
	int src_community_has_been_used;
	int dest_community_has_been_used;

	*stop_scanning_position = exchange_rankings_number;

	if(*selected = (short*) malloc(exchange_rankings_number * sizeof(short))) {
		if(used_community = (int*) malloc(cd->n * sizeof(int))) {
			if(used_node = (int*) malloc(cd->n * sizeof(int))) {

				// pragma omp
				memset(used_community, 0 , cd->n * sizeof(int));
				memset(used_node, 0 , cd->n * sizeof(int));

				while(used_communities < cd->n && current < exchange_rankings_number) {
					node_has_been_used = *(used_node + (exchange_rankings_sorted+current)->node);
					src_community_has_been_used = *(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node));
					dest_community_has_been_used = *(used_community + (exchange_rankings_sorted+current)->dest);

					if(!node_has_been_used && !src_community_has_been_used && !dest_community_has_been_used) {
						// Source and destination communities weren't used yet by any exchange

						// Exchange selected
						*(*selected + current) = 1;
						// Vertex has been used
						*(used_node + (exchange_rankings_sorted+current)->node) = 1;
						// Source community used
						*(used_community + *(cd->vertex_community + (exchange_rankings_sorted+current)->node)) = 1;
						// Destination community used
						*(used_community + (exchange_rankings_sorted+current)->dest) = 1;

						used_communities += 2;

						// Last selected exchange updated. Stop scanning selected array at position current + 1.
						*stop_scanning_position = current + 1;
					} else {
						// Source and destination communities were already used by any exchange

						// Exchange selected
						*(*selected + current) = 0;
					}

					current++;
				}

				free(used_community);
				free(used_node);
			} else {
				printf("Cannot allocate used node array!");
				free(used_community);
				free(*selected);

				return 0;
			}
		} else {
			printf("Cannot allocate used community array!");
			free(*selected);

			return 0;
		}
	} else {
		printf("Cannot allocate selected array!");

		return 0;
	}

	return 1;
}

int community_exchange_compare (const void * a, const void * b)
{
	double result = ( ((community_exchange*)a)->modularity_delta - ((community_exchange*)b)->modularity_delta );

	if(result > 0)
		return -1;
	else if (result == 0)
		return 0;
	else
		return 1;
}

// If threads = 1 output sorted wont be used
void community_exchange_parallel_quick_sort_merge(community_exchange *exchange_rankings, int first_partition_start, int first_partition_end, int second_partition_start, int second_partition_end,community_exchange *output_sorted) {
	int i = first_partition_start;
	int j = second_partition_start;

	int compare_result;
	int copy_to = first_partition_start;

	while(i < first_partition_end && j < second_partition_end) {
		compare_result = -community_exchange_compare(exchange_rankings+i, exchange_rankings+j);

		if(compare_result > 0) {
			// First greater then second
			*(output_sorted+copy_to) = *(exchange_rankings+i);

			i++;
		} else if (compare_result == 0) {
			// First and second are equal
			*(output_sorted+copy_to) = *(exchange_rankings+i);
			copy_to++;
			*(output_sorted+copy_to) = *(exchange_rankings+j);

			i++;
			j++;

		} else {
			// Second greater than first
			*(output_sorted+copy_to) = *(exchange_rankings+j);

			j++;
		}

		copy_to++;
	}

	// If first partition was not fully copied
	for(;i < first_partition_end;i++) {
		*(output_sorted+copy_to) = *(exchange_rankings+i);
		copy_to++;
	}

	// If second partition was not fully copied
	for(;j < second_partition_end;j++) {
		*(output_sorted+copy_to) = *(exchange_rankings+j);
		copy_to++;
	}
}

int community_exchange_parallel_quick_sort_main(community_exchange *exchange_rankings, int total_exchanges, execution_settings *settings, community_exchange **output_sorted) {
	int i;
	int partitions_number;
	int partition_size;
	int base_partition_size;

	int first_partition_start;
	int first_partition_end;
	int second_partition_start;
	int second_partition_end;

	int number_of_threads;

	int level;

	if(!exchange_rankings) {
		printf("community_exchange_parallel_quick_sort_main - exchange_rankings is NULL!");

		return 0;
	}

	*output_sorted = NULL;

	number_of_threads = omp_get_max_threads();

	partitions_number = lower_power_of_2(number_of_threads);

	if(settings->execution_settings_parallel_partitions_higher_power_of_2 && partitions_number < number_of_threads)
		partitions_number *= 2;

	// Makes sure that base partition size isn't smaller than MINIMUM_PARTITION_SIZE
	while(partitions_number > 1 && total_exchanges / partitions_number < MINIMUM_PARTITION_SIZE) {
		partitions_number /= 2;
	}

	if(partitions_number < 1) {
		printf("Partitions number smaller than zero: can't sort.\n");

		return 0;
	}

	base_partition_size = total_exchanges / partitions_number;

	if(total_exchanges > base_partition_size * partitions_number)
		base_partition_size++;

#ifndef VERBOSE_TABLE
	if(settings->verbose) {
		printf("Starting sorting of %d node transfers using a maximum of %d threads.\n"
				"Number of partitions: %d. Base partition size: %d.\n"
				"Minimum allowed partition size: %d\n",
				total_exchanges, number_of_threads,
				partitions_number, base_partition_size,
				MINIMUM_PARTITION_SIZE);
	}
#endif

//	printf("\nSORTING EXCHANGES\n\nTotal exchanges: %d\nPartitions number: %d\nBase partition size: %d\nNumber of threads: %d\n\n", total_exchanges, partitions_number, base_partition_size, number_of_threads);
	// Divide the input array in partition and qsort them in parallel

#pragma omp parallel for default(shared) private(i,partition_size) schedule(static, 1) num_threads(partitions_number)
	for(i = 0; i < partitions_number; i++)
	{
		if(i == (partitions_number - 1) && base_partition_size * partitions_number > total_exchanges)
			partition_size = total_exchanges - base_partition_size * (partitions_number - 1);
		else
			partition_size = base_partition_size;

//#ifdef _OPENMP
//		printf("Thread #%d - sorting exchanges from index %d to %d.\n", omp_get_thread_num(), i * base_partition_size, i * base_partition_size + partition_size - 1);
//#endif

		qsort(exchange_rankings + i * base_partition_size, partition_size, sizeof(community_exchange), community_exchange_compare);
	}

	if(partitions_number > 1) {

		if(!(*output_sorted = (community_exchange *) malloc(total_exchanges * sizeof(community_exchange)))) {
			printf("Couldn't allocate space needed to merge partitions in sorting of community exchanges!");

			return 0;
		}

		level = 0;

		while(partitions_number != 1) {

			// Merge subsections of the array in parallel
			// TODO May use num_treads(partitions_number/2) if needed
#pragma omp parallel for schedule(static, 1) num_threads(partitions_number) default(shared) private(i, first_partition_start, second_partition_start, first_partition_end, second_partition_end)
			for(i = 0; i < partitions_number; i+=2) {

				first_partition_start = i * base_partition_size;
				second_partition_start = first_partition_end = (i + 1) * base_partition_size;

				if(i == (partitions_number - 2) && partitions_number * base_partition_size > total_exchanges)
					second_partition_end = second_partition_start + total_exchanges - base_partition_size * (partitions_number - 1);
				else
					second_partition_end = (i + 2) * base_partition_size;

//#ifdef _OPENMP
//				printf("Thread #%d - merging sorted exchanges (%d - %d) and (%d - %d).\n", omp_get_thread_num(), first_partition_start, first_partition_end - 1, second_partition_start, second_partition_end - 1);
//#endif

				// Choose the correct layer containing the correct subsections to sort
				if(level % 2)
					community_exchange_parallel_quick_sort_merge(*output_sorted, first_partition_start, first_partition_end, second_partition_start, second_partition_end, exchange_rankings);
				else
					community_exchange_parallel_quick_sort_merge(exchange_rankings, first_partition_start, first_partition_end, second_partition_start, second_partition_end, *output_sorted);

			}

			partitions_number /= 2;
			base_partition_size *= 2;
			level++;
		}

		if(level % 2 == 0) {
			// Output is contained in exchange_rankings, output_sorted is no more needed

			free(*output_sorted);
			*output_sorted = NULL;

		}
	}

	return 1;
}
