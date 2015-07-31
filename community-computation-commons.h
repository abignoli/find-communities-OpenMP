/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef COMMUNITY_COMPUTATIONS_COMMONS_H
#define COMMUNITY_COMPUTATIONS_COMMONS_H

#include <stdio.h>

int community_vector_init(int **community_vector, int size);

int output_save_communities(FILE *output_communities_file,int *community_vector, int size);

int output_save_community_graph(FILE *output_graphs_file, dynamic_weighted_graph *output_graph, int phase_counter);

#endif
