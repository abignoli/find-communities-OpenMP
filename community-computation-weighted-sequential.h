#ifndef COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H
#define COMMUNITY_COMPUTATION_WEIGHTED_SEQUENTIAL_H

typedef struct dynamic_weighted_graph dynamic_weighted_graph;
typedef struct execution_settings execution_settings;

double sequential_phase_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector);

double sequential_find_communities_weighted(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector);

#endif
