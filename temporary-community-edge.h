#ifndef TEMPORARY_COMMUNITY_EDGE_H
#define TEMPORARY_COMMUNITY_EDGE_H

#include "sorted-linked-list.h"

typedef struct temporary_community_edge {
	int self_loop;
	sorted_linked_list sll;
} temporary_community_edge;

void temporary_community_edge_init(temporary_community_edge *tce);

void temporary_community_edge_free(temporary_community_edge *tce);

#endif
