/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include <time.h>

#define PRINTING_UTILITY_STARS "\n\n********************************************************************************\n\n"
#define PRINTING_SORT_SELECT_TIME_TABLE_HEADER "\n\nIteration       Initial Modularity      Final Modularity      Gain           Node Scanning       Edge Compression      Edge Sorting        Edge Selection       Applying Changes       Iteration Duration\n\n"
#define PRINTING_SORT_SELECT_TIME_TABLE_VALUES_PERCENT "%03d            %.6f                %.6f              %.6f        %5.2f%%             %5.2f%%                %5.2f%%              %5.2f%%                %4.2f%%                  %.6fs\n"
#define PRINTING_SORT_SELECT_TIME_TABLE_VALUES "                                                                             %.6fs           %.6fs             %.6fs           %.6fs            %.6fs              %.6fs\n\n"

#define PRINTING_SEQUENTIAL_TIME_TABLE_HEADER "\n\nIteration       Initial Modularity      Final Modularity      Gain            Neighbor Scanning      Select Neighbor      Applying Changes      Iteration Duration\n\n"
#define PRINTING_SEQUENTIAL_TIME_TABLE_VALUES_PERCENT "%03d            %.6f                %.6f              %.6f        %5.2f%%                  %5.2f%%                %5.2f%%                 %.6fs\n"
#define PRINTING_SEQUENTIAL_TIME_TABLE_VALUES "%03d            %.6f                %.6f              %.6f        %.6fs                 %.6fs               %.6fs              %.6fs\n\n"

#define PRINTING_UTILITY_EQUALS "\n\n================================================================================\n\n"
#define PRINTING_UTILITY_DASHES "\n\n--------------------------------------------------------------------------------\n\n"
#define PRINTING_UTILITY_SPARSE_DASHES "\n\n -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n\n"
#define PRINTING_UTILITY_INDENT_TITLE "                    "

#define PRINTING_NOT_YET_IMPLEMENTED "Not yet implemented!\n"
#define PRINTING_NOT_ALGORITHM_ERRORS "Execution of the algorithm terminated with errors!\n"

int min(int a, int b);

int max(int a, int b);

inline int same(int x, int y);

float merge_average(float first_average, int first_number_of_averaged_elements, float second_average, int second_number_of_averaged_elements);

// Extremes included
int in_range(int x, int start, int end);

double delta_seconds(clock_t begin, clock_t end);

#endif
