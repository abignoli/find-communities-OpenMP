/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include "neighbor-computation-package.h"

#define INIT_CHUNK_SIZE 50

int neighbor_computation_package_init(neighbor_computation_package *ncp, int size) {
	int i;

	if(!(ncp->communities = (int *) malloc(size * sizeof(int)))) {
		printf("neighbor_computation_package_init - Could not allocate communities array!\n");
		return 0;
	}

	if(!(ncp->community_list = (int *) malloc(size * sizeof(int)))) {
		printf("neighbor_computation_package_init - Could not allocate community list array!\n");
		free(ncp->communities);
		return 0;
	}

	ncp->count = 0;

#pragma omp parallel for default(shared) private(i) schedule(dynamic,INIT_CHUNK_SIZE)
	for(i = 0; i < size; i++)
		*(ncp->communities + i) = 0;

	return 1;
}

void neighbor_computation_package_clean(neighbor_computation_package *ncp, int size) {

	while(ncp->count > 0) {
		*(ncp->communities + *(ncp->community_list + ncp->count - 1)) = 0;

		ncp->count--;
	}
}

int neighbor_computation_package_insert(neighbor_computation_package *ncp, int community, int weight) {
	// No range check is performed. Output is undefined if given community is not in the range of the size of the package

	if(*(ncp->communities + community) == 0) {
		// Community was never inserted, we need to create an entry in the community list (array) to be able to cleanup efficiently later
		*(ncp->community_list + ncp->count) = community;
		ncp->count++;
	}

	*(ncp->communities + community) += weight;

	return 1;
}

// Pops a community in the list copying its values in the input parameters, and clean its entry both from list and array
int neighbor_computation_package_pop_and_clean(neighbor_computation_package *ncp, int *community, int *k_i_in) {
	if(ncp->count == 0)
		// Nothing to pop
		return 0;

	// Retrieve
	*community = *(ncp->community_list + ncp->count - 1);
	*k_i_in = *(ncp->communities + *community);

	// Clean
	*(ncp->communities + *community) = 0;
	ncp->count--;

	return 1;
}



