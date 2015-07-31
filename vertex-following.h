/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef VERTEX_FOLLOWING_H
#define VERTEX_FOLLOWING_H

#define VERTEX_FOLLOWING_NODE_CHUNK 50

int pre_compute_vertex_following(dynamic_weighted_graph *dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, phase_execution_briefing *briefing);

#endif
