/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "parse-args.h"
#include "execution-settings.h"
#include "dynamic-graph.h"
#include "dynamic-weighted-graph.h"
#include "utilities.h"
#include <stdio.h>
#include <string.h>
#include "shared-graph.h"

#define METIS_MAXIMUM_GRAPH_DESCRIPTOR_LENGTH 16
#define METIS_MAXIMUM_DIGITS_PER_NUMBER 16

int dynamic_weighted_graph_parse_file(dynamic_weighted_graph *dg, char *filename) {
	FILE * graph_file;
	int src, dest, weight;

	if(!dynamic_weighted_graph_init(dg,DEFAULT_INIT_NODES_SIZE))
		return 0;

	// Read file - Insert edges
	if(graph_file = fopen(filename,"r")) {
		while(fscanf(graph_file, "%d %d %d", &src, &dest, &weight) == 3)
			if(!dynamic_weighted_graph_insert(dg,src,dest, weight)) {
				printf("Could not insert edge %d - %d!\n", src, dest);
				return 0;
			}
	} else {
		printf("Could not open file: %s!\n", filename);

		return 0;
	}

	// Reduce to optimal size
	if(!dynamic_weighted_graph_reduce(dg)) {
		printf("Could not reduce graph size!\n");

		return 0;
	}

	return 1;
}

// Assign given weight to each edge
int dynamic_weighted_graph_parse_not_weighted_file(dynamic_weighted_graph *dwg, char *filename, int weight) {
	FILE * graph_file;
	int src, dest;

	if(!dynamic_weighted_graph_init(dwg,DEFAULT_INIT_NODES_SIZE))
		return 0;

	// Read file - Insert edges
	if(graph_file = fopen(filename,"r")) {
		while(fscanf(graph_file, "%d %d", &src, &dest) == 2)
			if(!dynamic_weighted_graph_insert(dwg,src,dest, weight)) {
				printf("Could not insert edge %d - %d!\n", src, dest);
				return 0;
			}
	} else {
		printf("Could not open file: %s!\n", filename);

		return 0;
	}

	// Reduce to optimal size
	if(!dynamic_weighted_graph_reduce(dwg)) {
		printf("Could not reduce graph size!\n");

		return 0;
	}

	return 1;
}

// Assumption in file parsing: No duplicates, edge weights and numbers positive
int dynamic_graph_parse_file(dynamic_graph *dg, char *filename) {
	FILE * graph_file;
	int src, dest, weight;

	if(!dynamic_graph_init(dg,DEFAULT_INIT_NODES_SIZE))
		return 0;

	// Read file - Insert edges
	if(graph_file = fopen(filename,"r")) {
		while(fscanf(graph_file, "%d %d", &src, &dest) == 2)
			if(!dynamic_graph_insert(dg,src,dest)) {
				printf("Could not insert edge %d - %d!\n", src, dest);
				return 0;
			}
	} else {
		printf("Could not open file: %s!\n", filename);

		return 0;
	}

	// Reduce to optimal size
	if(!dynamic_graph_reduce(dg)) {
		printf("Could not reduce graph size!\n");

		return 0;
	}

	return 1;
}

int parse_input(dynamic_graph *dg, dynamic_weighted_graph *dwg, execution_settings *settings) {
	int valid = 1;

	printf(PRINTING_UTILITY_STARS);
	printf("Parsing input graph...\n");

//	if(settings->graph_type == WEIGHTED) {
//		if(!dynamic_weighted_graph_parse_file(dwg, settings->input_file))
//			valid = 0;
//	} else {
//		// TODO Use actual not weighted graph
//		settings->graph_type = WEIGHTED;
//
//		if(!dynamic_weighted_graph_parse_not_weighted_file(dwg, settings->input_file, DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES))
//			valid = 0;
//	}

	switch(settings->input_file_format) {
	case FILE_FORMAT_EDGE_LIST_NOT_WEIGHTED:
				// TODO Use actual not weighted graph
				settings->graph_type = WEIGHTED;

				if(!dynamic_weighted_graph_parse_not_weighted_file(dwg, settings->input_file, DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES))
					valid = 0;
				break;
	case FILE_FORMAT_EDGE_LIST_WEIGHTED:
				settings->graph_type = WEIGHTED;

				if(!dynamic_weighted_graph_parse_file(dwg, settings->input_file))
					valid = 0;
				break;
	case FILE_FORMAT_METIS:
		if(!read_metis_format(dg,dwg,settings))
			valid = 0;
		break;
	}

	if(!valid){
		printf("Failed to read input file: %s\n", settings->input_file);
		return 0;
	}

	if(settings->verbose) {
		if(settings->graph_type == NOT_WEIGHTED)
			dynamic_graph_print(*dg);
		else
			dynamic_weighted_graph_print(*dwg);
	}

	printf("Done.");

	return 1;
}

int read_metis_format_not_weighted_to_weighted(dynamic_weighted_graph *dwg, FILE *graph_file, execution_settings *settings) {
    char ch;
    int current_number_digit = 0;
    int src = 0;
    int target_node;

    // Assuming METIS_MAXIMUM_DIGITS_PER_NUMBER digits are enough for the index of any node in the input graph
    char current_number_as_string[METIS_MAXIMUM_DIGITS_PER_NUMBER + 1];

    ch = getc(graph_file);
    while ( ch != EOF ) {
        if ( ch == '\n' || ch == ' '){
        	// Current number, if existing, has ended
        	if(current_number_digit > 0) {
        		current_number_as_string[current_number_digit] = '\0';

        		sscanf(current_number_as_string,"%d",&target_node);

        		// In Metis format node indexes start from one
        		target_node--;

        		dynamic_weighted_graph_insert_force_directed(dwg,src,target_node, DEFAULT_WEIGHT_FOR_NOT_WEIGHTED_EDGES);

        		current_number_digit = 0;
        	}
        } else {
        	current_number_as_string[current_number_digit] = ch;
            current_number_digit++;

            if(current_number_digit >= METIS_MAXIMUM_DIGITS_PER_NUMBER) {
            	printf("Parsing input file - maximum digits per number exceeded!\n");

            	return 0;
            }

        }

        if(ch == '\n') {
        	src++;
        }

        ch = getc (graph_file);
    }

    return 1;
}

int read_metis_format(dynamic_graph *dg, dynamic_weighted_graph *dwg, execution_settings *settings) {
	FILE * graph_file;
	int number_of_nodes, number_of_edges;
	char graph_descriptor[METIS_MAXIMUM_GRAPH_DESCRIPTOR_LENGTH + 1];
	int valid = 1;
	char c;

	int graph_descriptor_index;

	// Open file
	if(!(graph_file = fopen(settings->input_file,"r"))) {
		printf("Could not open file: %s!\n", settings->input_file);

		return 0;
	}

	if(!(fscanf(graph_file, "%d %d", &number_of_nodes, &number_of_edges) == 2)) {
		printf("Invalid graph header!");

		valid = 0;
	}


	graph_descriptor_index = 0;
	c = getc(graph_file);
	while(c == ' ')
		c = getc(graph_file);

	if(c != '\n') {
		// There is a graph descriptor to be read
		while(c != '\n' && c != ' ') {
			if(graph_descriptor_index == METIS_MAXIMUM_GRAPH_DESCRIPTOR_LENGTH - 1) {
				graph_descriptor[METIS_MAXIMUM_GRAPH_DESCRIPTOR_LENGTH - 1] = '\0';
				printf("Could not parse all graph header! So far '%s'\n", graph_descriptor);

				valid = 0;
			}

			graph_descriptor[graph_descriptor_index] = c;
			graph_descriptor_index++;

			c = getc(graph_file);
		}
	}

	graph_descriptor[graph_descriptor_index] = '\0';

	while(c == ' ')
		c = getc(graph_file);

	if(c != '\n') {
		printf("Graph headers composed by four elements are not supported yet!\n");

		return 0;
	}


	if(valid) {
		if(strcmp("0",graph_descriptor) == 0) {
			// Graph is not weighted
			// TODO Use proper function instead of transforming it into weighted
			settings->graph_type = WEIGHTED;
			dynamic_weighted_graph_init(dwg, number_of_nodes);
			valid = read_metis_format_not_weighted_to_weighted(dwg, graph_file, settings);
		}
		else if(strcmp("1",graph_descriptor) == 0) {
			// Graph is weighted
			printf("Graph descriptor '%s' not supported yet!", graph_descriptor);

			valid = 0;
		} else {
			printf("Graph descriptor '%s' not supported yet!", graph_descriptor);

			valid = 0;
		}

		fclose(graph_file);
	}

	if(valid) {
		if(settings->graph_type == NOT_WEIGHTED)
			dynamic_graph_reduce(dg);
		else
			dynamic_weighted_graph_reduce(dwg);
	}

	return valid;
}

