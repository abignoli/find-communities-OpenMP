/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef NEIGHBOR_COMPUTATION_PACKAGE_H
#define NEIGHBOR_COMPUTATION_PACKAGE_H

#define INIT_CHUNK_SIZE 50

typedef struct neighbor_computation_package {
	int *communities;
	int *community_list;
	int count;
} neighbor_computation_package;

int neighbor_computation_package_init(neighbor_computation_package *ncp, int size);

void neighbor_computation_package_clean(neighbor_computation_package *ncp, int size);

int neighbor_computation_package_insert(neighbor_computation_package *ncp, int community, int weight);

int neighbor_computation_package_community_list_insert(neighbor_computation_package *ncp, int key);

// Pops a community in the list copying its values in the input parameters, and clean its entry both from list and array
int neighbor_computation_package_pop_and_clean(neighbor_computation_package *ncp, int *community, int *k_i_in);

#endif
