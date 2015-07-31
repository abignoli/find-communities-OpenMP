/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef ALGORITHM_EXECUTOR_H
#define ALGORITHM_EXECUTOR_H

int find_communities(dynamic_graph *dg ,dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, algorithm_execution_briefing *briefing);

#endif
