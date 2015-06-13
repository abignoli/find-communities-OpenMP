#include "execution-briefing.h"
#include <stdio.h>
#include "utilities.h"

void execution_briefing_print(execution_briefing *briefing) {
	printf(PRINTING_UTILITY_STARS);

	printf("Execution briefing:\n\n");
	printf("\tExecution successful:                                %s\n", (briefing->execution_successful ? "Yes" : "No"));
	printf("\tNumber of algorithm runs:                            %d\n", briefing->performed_runs);
	printf("\tAverage modularity obtained:                         %f\n", briefing->output_modularity);
	printf("\tAveraged execution time:                             %fs\n", briefing->execution_time);
	printf("\tMinimum execution time:                              %fs\n", briefing->minimum_execution_time);
	printf("\tMinimum clock execution time:                        %fs\n", briefing->minimum_clock_execution_time);
	printf("\tAveraged sum of execution time over all threads:     %fs\n", briefing->clock_execution_time);
	printf("\tTotal execution time:                                %fs\n", briefing->global_execution_time);
}
