/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef EXECUTION_HANDLER_H
#define EXECUTION_HANDLER_H

typedef struct execution_briefing execution_briefing;

int execute_community_detection(dynamic_graph *input_dg, dynamic_weighted_graph *input_dwg, execution_settings *settings, dynamic_weighted_graph **community_graph, int **community_vector, execution_briefing *briefing);
#endif
