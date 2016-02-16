/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "temporary-community-edge.h"

void temporary_community_edge_init(temporary_community_edge *tce){
	tce->self_loop = 0;
	sorted_linked_list_init(&(tce->sll));
}

void temporary_community_edge_free(temporary_community_edge *tce){
	sorted_linked_list_free(&(tce->sll));
}
