#ifndef COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H
#define COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H

typedef struct dynamic_weighted_graph dynamic_weighted_graph;

double phase_weighted_sequential(dynamic_weighted_graph *dwg, double minimum_improvement, dynamic_weighted_graph **community_graph, int **community_vector);

#endif
