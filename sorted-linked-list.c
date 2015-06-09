#include "community-development.h"
#include "sorted-linked-list.h"
#include <stdio.h>

void sorted_linked_list_init(sorted_linked_list *sll) {
	sll->head = NULL;
}

int sorted_linked_list_insert(sorted_linked_list *sll, int community, int k_i_in) {
	sorted_linked_list_elem *current;
	sorted_linked_list_elem *previous;

	sorted_linked_list_elem *new_elem;

	if(!sll) {
		printf("Sorted linked list null!");

		return 0;
	}

	current = sll->head;
	previous = NULL;
	while(current && current->community < community) {
		previous = current;
		current = current->next;
	}

	// The community was already in the list
	if(current && current->community == community)
		current->k_i_in += k_i_in;
	else if(new_elem = (sorted_linked_list_elem*) malloc(sizeof(sorted_linked_list_elem))) {
		new_elem->community = community;
		new_elem->k_i_in = k_i_in;

		// If the element should be placed as a new head
		if(!previous) {
			sll->head = new_elem;
			new_elem->next = current;
		} else {
			new_elem->next = current;
			previous->next = new_elem;
		}
	} else {
		printf("Out of memory!");

		return 0;
	}

	return 1;
}

void sorted_linked_list_free(sorted_linked_list *sll) {
	sorted_linked_list_elem *current;
	sorted_linked_list_elem *tmp_next;

	if(sll) {
		current = sll->head;
		while(current) {
			tmp_next = current-> next;
			free(current);
			current = tmp_next;
		}

		sll->head = NULL;
	}
}
