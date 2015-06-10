#ifndef COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H
#define COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H

typedef struct dynamic_weighted_graph dynamic_weighted_graph;

double sequential_phase_weighted(dynamic_weighted_graph *dwg, double minimum_improvement, dynamic_weighted_graph **community_graph, int **community_vector);

double sequential_find_communities_weighted(dynamic_weighted_graph *dwg, double minimum_phase_improvement, double minimum_iteration_improvement, char *output_communities_filename, char *output_graphs_filename, dynamic_weighted_graph **community_graph, int **community_vector);

#endif
