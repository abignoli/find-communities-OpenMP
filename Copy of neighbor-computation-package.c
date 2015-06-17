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

#pragma omp parallel for default(shared) private(i) schedule(dynamic,INIT_CHUNK_SIZE)
	for(i = 0; i < size; i++)
		*(ncp->communities + i) = 0;

	ncp->community_list = NULL;

	return 1;
}

void neighbor_computation_package_clean(neighbor_computation_package *ncp, int size) {
	community_list_elem * tmp;

	while(ncp->community_list != NULL) {
		*(ncp->communities + (ncp->community_list)->key) = 0;
		tmp = ncp->community_list;
		ncp->community_list = (ncp->community_list)->next;
		free(tmp);
	}
}

int neighbor_computation_package_community_list_insert(neighbor_computation_package *ncp, int key) {
	community_list_elem *new_elem;
	if(!(new_elem = (community_list_elem *) malloc(sizeof(community_list_elem)))) {
		printf("community_list_insert - Could not allocate new element!\n");
		return 0;
	}

	new_elem->key = key;
	new_elem->next = ncp->community_list;
	ncp->community_list = new_elem;

	return 1;
}

int neighbor_computation_package_insert(neighbor_computation_package *ncp, int community, int weight) {
	// No range check is performed. Output is undefined if given community is not in the range of the size of the package

	if(*(ncp->communities + community) == 0) {
		// Community was never inserted, we need to create an entry in the list to be able to cleanup efficiently later
		if(!neighbor_computation_package_community_list_insert(ncp,community)) {
			printf("neighbor_computation_package_insert - Could not allocate new element!\n");
			return 0;
		}
	}

	*(ncp->communities + community) += weight;

	return 1;
}

// Pops a community in the list copying its values in the input parameters, and clean its entry both from list and array
int neighbor_computation_package_pop_and_clean(neighbor_computation_package *ncp, int *community, int *k_i_in) {
	community_list_elem * tmp;

	if(ncp->community_list == NULL)
		// Nothing to pop
		return 0;

	tmp = ncp->community_list;

	// Retrieve
	*community = tmp->key;
	*k_i_in = *(ncp->communities + tmp->key);

	// Clean
	*(ncp->communities + tmp->key) = 0;
	ncp->community_list = tmp->next;
	free(tmp);

	return 1;
}



