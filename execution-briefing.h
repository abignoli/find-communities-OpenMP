#ifndef EXECUTION_BRIEFING_H
#define EXECUTION_BRIEFING_H

typedef struct execution_briefing {
	int performed_runs;
	int execution_successful;
	double output_modularity;
	double execution_time;
	double clock_execution_time;

	double minimum_execution_time;
	double minimum_clock_execution_time;

	// Includes file IO, not relevant for scalability measures!
	double global_execution_time;
} execution_briefing;

typedef struct algorithm_execution_briefing {
	int execution_successful;
	double output_modularity;
	double execution_time;
	double clock_execution_time;

	// Includes file IO, not relevant for scalability measures!
	double global_execution_time;
} algorithm_execution_briefing;

//typedef struct phase_execution_briefing {
//	int execution_successful;
//	double output_modularity;
//	double execution_time;
//	double clock_execution_time;
//} algorithm_execution_briefing;

void execution_briefing_print(execution_briefing *briefing);

#endif
