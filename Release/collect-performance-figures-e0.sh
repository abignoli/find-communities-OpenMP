#!/bin/bash

INPUT_FILE=$1
INPUT_FORMAT=$2
OUTPUT_FILE=$3
BENCHMARK_RUNS=$4
MAX_THREADS=$5

echo "Collecting performance figures in $OUTPUT_FILE using $INPUT_FILE as input graph (-e 0 option)"
echo "Collecting performance figures in $OUTPUT_FILE using $INPUT_FILE as input graph (-e 0 option)" > $OUTPUT_FILE

echo "Running sequential version"
OUTPUT="$(./find-communities-OpenMP $INPUT_FILE -f $INPUT_FORMAT -b $BENCHMARK_RUNS -a 0 | grep -E 'Algorithm version|Number of threads:                |Average execution time|Minimum execution time' )"
echo $OUTPUT >> $OUTPUT_FILE

printf "\n\n"

echo "Running parallel (Sort & Merge) version with 1 thread"
echo "Running parallel (Sort & Merge) version with 1 thread" >> $OUTPUT_FILE
OUTPUT="$(./find-communities-OpenMP $INPUT_FILE -f 2 -b $BENCHMARK_RUNS -a 1 -t 1 -e 0 | grep -E 'Algorithm version|Number of threads:                |Average execution time|Minimum execution time' )"
echo $OUTPUT >> $OUTPUT_FILE
SINGLE_THREAD_TIME_AVERAGE="$(echo $OUTPUT | awk '{print $14}')"
SINGLE_THREAD_TIME_AVERAGE=${SINGLE_THREAD_TIME_AVERAGE::-1}
echo "Average time: " $SINGLE_THREAD_TIME_AVERAGE

SINGLE_THREAD_TIME_MINIMUM="$(echo $OUTPUT | awk '{print $18}')"
SINGLE_THREAD_TIME_MINIMUM=${SINGLE_THREAD_TIME_MINIMUM::-1}
echo "Minimum time: " $SINGLE_THREAD_TIME_MINIMUM

printf "\n\n"

for i in $(seq 2 $MAX_THREADS);
do
	echo "Running parallel (Sort & Merge) version with $i threads"
	echo "Running parallel (Sort & Merge) version with $i threads" >> $OUTPUT_FILE
	OUTPUT="$(./find-communities-OpenMP $INPUT_FILE -f $INPUT_FORMAT -b $BENCHMARK_RUNS -a 1 -t $i -e 0 | grep -E 'Algorithm version|Number of threads:                |Average execution time|Minimum execution time' )"
	echo $OUTPUT >> $OUTPUT_FILE
	THREADS_TIME="$(echo $OUTPUT | awk '{print $14}')"
	THREADS_TIME=${THREADS_TIME::-1}
	echo "Average time: " $THREADS_TIME
	COMPUTATION="scale=10; $SINGLE_THREAD_TIME_AVERAGE / $THREADS_TIME * 100"
	SPEEDUP_AVERAGE="$(echo "$COMPUTATION" | bc)"
	
	THREADS_TIME="$(echo $OUTPUT | awk '{print $18}')"
	THREADS_TIME=${THREADS_TIME::-1}
	echo "Minimum time: " $THREADS_TIME
	COMPUTATION="scale=10; $SINGLE_THREAD_TIME_MINIMUM / $THREADS_TIME * 100"
	SPEEDUP_MINIMUM="$(echo "$COMPUTATION" | bc)"
	
	echo $i "threads --> Speedup = Computed on average: " $SPEEDUP_AVERAGE "% - Computed on minimum = " $SPEEDUP_MINIMUM "%"
	echo $i "threads --> Speedup = Computed on average: " $SPEEDUP_AVERAGE "% - Computed on minimum = " $SPEEDUP_MINIMUM "%" >> $OUTPUT_FILE
	printf "\n\n"
done

echo "Running parallel (Naive Partitioning) version with 1 thread"
echo "Running parallel (Naive Partitioning) version with 1 thread" >> $OUTPUT_FILE
OUTPUT="$(./find-communities-OpenMP $INPUT_FILE -f 2 -b $BENCHMARK_RUNS -a 2 -t 1 | grep -E 'Algorithm version|Number of threads:                |Average execution time|Minimum execution time' )"
echo $OUTPUT >> $OUTPUT_FILE
SINGLE_THREAD_TIME_AVERAGE="$(echo $OUTPUT | awk '{print $16}')"
SINGLE_THREAD_TIME_AVERAGE=${SINGLE_THREAD_TIME_AVERAGE::-1}
echo "Average time: " $SINGLE_THREAD_TIME_AVERAGE

SINGLE_THREAD_TIME_MINIMUM="$(echo $OUTPUT | awk '{print $20}')"
SINGLE_THREAD_TIME_MINIMUM=${SINGLE_THREAD_TIME_MINIMUM::-1}
echo "Minimum time: " $SINGLE_THREAD_TIME_MINIMUM

printf "\n\n"

for i in $(seq 2 $MAX_THREADS);
do
	echo "Running parallel (Naive Partitioning) version with $i threads"
	echo "Running parallel (Naive Partitioning) version with $i threads" >> $OUTPUT_FILE
	OUTPUT="$(./find-communities-OpenMP $INPUT_FILE -f $INPUT_FORMAT -b $BENCHMARK_RUNS -a 2 -t $i | grep -E 'Algorithm version|Number of threads:                |Average execution time|Minimum execution time' )"
	echo $OUTPUT >> $OUTPUT_FILE
	THREADS_TIME="$(echo $OUTPUT | awk '{print $16}')"
	THREADS_TIME=${THREADS_TIME::-1}
	echo "Average time: " $THREADS_TIME
	COMPUTATION="scale=10; $SINGLE_THREAD_TIME_AVERAGE / $THREADS_TIME * 100"
	SPEEDUP_AVERAGE="$(echo "$COMPUTATION" | bc)"
	
	THREADS_TIME="$(echo $OUTPUT | awk '{print $20}')"
	THREADS_TIME=${THREADS_TIME::-1}
	echo "Minimum time: " $THREADS_TIME
	COMPUTATION="scale=10; $SINGLE_THREAD_TIME_MINIMUM / $THREADS_TIME * 100"
	SPEEDUP_MINIMUM="$(echo "$COMPUTATION" | bc)"
	
	echo $i "threads --> Speedup = Computed on average: " $SPEEDUP_AVERAGE "% - Computed on minimum = " $SPEEDUP_MINIMUM "%"
	echo $i "threads --> Speedup = Computed on average: " $SPEEDUP_AVERAGE "% - Computed on minimum = " $SPEEDUP_MINIMUM "%" >> $OUTPUT_FILE
	printf "\n\n"
done
