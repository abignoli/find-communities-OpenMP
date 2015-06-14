#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <stdio.h>

int dynamic_graph_parse_file(dynamic_graph *dg, char *filename);

int dynamic_weighted_graph_parse_file(dynamic_weighted_graph *dg, char *filename);

int dynamic_weighted_graph_parse_not_weighted_file(dynamic_weighted_graph *dwg, char *filename, int weight);

int parse_input(dynamic_graph *dg, dynamic_weighted_graph *dwg, execution_settings *settings);

int read_metis_format_not_weighted_to_weighted(dynamic_weighted_graph *dwg, FILE *graph_file, execution_settings *settings);

int read_metis_format(dynamic_graph *dg, dynamic_weighted_graph *dwg, execution_settings *settings);

#endif
