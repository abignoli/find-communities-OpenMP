/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef SORTED_LINKED_LIST_H
#define SORTED_LINKED_LIST_H

typedef struct sorted_linked_list_elem {
	int community;
	int k_i_in;
	struct sorted_linked_list_elem *next;
} sorted_linked_list_elem;

typedef struct sorted_linked_list {
	sorted_linked_list_elem *head;
} sorted_linked_list;

void sorted_linked_list_init(sorted_linked_list *sll);

int sorted_linked_list_insert(sorted_linked_list *sll, int community, int k_i_in);

void sorted_linked_list_free(sorted_linked_list *sll);

#endif
