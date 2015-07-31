/**
 * Author: Andrea Bignoli
 * E-mail: andrea.bignoli@gmail.com
 */

#include "utilities.h"
#include <time.h>

int min(int a, int b) {
	return a < b ? a : b;
}

int max(int a, int b) {
	return a > b ? a : b;
}

inline int same(int x, int y){
	return x == y ? 1 : 0;
}

int lower_power_of_2(int n) {
	int result;

	if(n <= 0)
		return -1;

	result = 1;

	while(result * 2 <= n) {
		result *= 2;
	}

	return result;
}

// Merge two averages
float merge_average(float first_average, int first_number_of_averaged_elements, float second_average, int second_number_of_averaged_elements) {
	return (first_average * first_number_of_averaged_elements + second_average * second_number_of_averaged_elements) / (first_number_of_averaged_elements + second_number_of_averaged_elements);
}

// Extremes included
int in_range(int x, int start, int end) {
	return x >= start && x <= end;
}

double delta_seconds(clock_t begin, clock_t end) {
	return (double)(end - begin) / CLOCKS_PER_SEC;
}
