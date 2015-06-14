#ifndef UTILITIES_H
#define UTILITIES_H

#define PRINTING_UTILITY_STARS "\n\n********************************************************************************\n\n"
#define PRINTING_UTILITY_DASHES "\n\n--------------------------------------------------------------------------------\n\n"
#define PRINTING_UTILITY_SPARSE_DASHES "\n\n -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -\n\n"
#define PRINTING_UTILITY_INDENT_TITLE "                    "

#define PRINTING_NOT_YET_IMPLEMENTED "Not yet implemented!\n"
#define PRINTING_NOT_ALGORITHM_ERRORS "Execution of the algorithm terminated with errors!\n"

int min(int a, int b);

int max(int a, int b);

inline int same(int x, int y);

float merge_average(float first_average, int first_number_of_averaged_elements, float second_average, int second_number_of_averaged_elements);

#endif
