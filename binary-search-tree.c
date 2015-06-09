typedef struct bynary_search_tree_elem {
	int community;
	int k_i_in;
} bynary_search_tree_elem;

/*
 * Derived from binary search tree implementation
 *
 * The element with the lowest community field will occupy the root of the tree.
 */
typedef struct bynary_search_tree{
	bynary_search_tree_elem *addr;
	int count;
	int size;
} bynary_search_tree;


int bynary_search_tree_init(bynary_search_tree *bst, int initSize)
{
	bst->size = lower_power_of_2(initSize) * 2;
	bst->count = 0;

	if(!(bst->addr = (bynary_search_tree_elem*) malloc(bst->size * sizeof(bynary_search_tree_elem))))
		return 0;

	return 1;
}

int bynary_search_tree_insert(bynary_search_tree *bst, int community, int k_i_in)
{
	bynary_search_tree_elem *tmp,*addr;
	bynary_search_tree_elem x;

	x.community = community;
	x.k_i_in = k_i_in;

	int i;

	if(bst->count < bst->size)
	{
		bynary_search_tree_ins(bst, x);
		return 1;
	} else {
		// Need more space, increase heap size
		if(bynary_search_tree_double_size(bst))
		{
			dynamic_heap_ins(bst, x);
			return 1;
		} else {
			printf("Can't insert data");

			return 0;
		}
	}
}

void bynary_search_tree_ins(bynary_search_tree *bst, bynary_search_tree_elem x)
{
	int c = 0;

	while(c < bst->size) {
		
	}
}

//// Use dynamic_heap_insert outside of this module!
//void bynary_search_tree_ins(bynary_search_tree *bst, bynary_search_tree_elem x)
//{
//	bynary_search_tree_elem tmp;
//	int c;
//
//	c = bst->count;
//	*(bst->addr+c) = x;
//
//	bst->count++;
//
//	while(c && (dynamic_heap_elem_cmp(bst->addr+(c-1)/2,bst->addr+c) > 0))
//	{
//		// While father > child, swap father and child --> O(logn)
//		tmp = *(bst->addr+c);
//		*(bst->addr+c) = *(bst->addr+(c-1)/2);
//		*(bst->addr+(c-1)/2) = tmp;
//		c = (c-1)/2;
//	}
//
//	return;
//}

void bynary_search_tree_free(bynary_search_tree *bst) {
	bst->count = 0;
	bst->size = 0;
	free(bst->addr);
}
/*
 * a.community >  b.community 	==> bynary_search_tree_elem_cmp(a,b) > 0
 * a.community == b.community 	==> bynary_search_tree_elem_cmp(a,b) = 0
 * a.community <  b.community 	==> bynary_search_tree_elem_cmp(a,b) < 0
 */
int bynary_search_tree_elem_cmp(void *a, void *b) {
	return ((bynary_search_tree_elem *)a)->community - ((bynary_search_tree_elem *)b)->community;
}

inline int father(int index) {
	if(index <= 0)
		return -1;

	return (index-1)/2;
}

inline int left_child(int index) {
	if(index <= 0)
		return -1;

	return index * 2 + 1;
}

inline int right_child(int index) {
	if(index <= 0)
		return -1;

	return index * 2 + 2;
}

int bynary_search_tree_double_size(bynary_search_tree *bst) {
	bynary_search_tree_elem *tmp,*addr;
	int i;

	if(tmp = (bynary_search_tree_elem *) malloc((bst->size * 2) * sizeof(bynary_search_tree_elem)) )
	{
		bst->size*=2;
		addr = bst->addr;
		for(i=0;i<bst->count;i++)
			*(tmp + i) = *(addr + i);
		free(addr); //Free previous array
		bst->addr = tmp;

		return 1;
	} else {
		printf("Run out of memory: can't insert data");

		return 0;
	}
}
