#!/bin/bash

echo "Collecting performance figures in performance-output.txt"

echo "Running sequential version"
./find-communities-OpenMP ../graphs/cond-mat-2003.graph -f 2 -b 2 -a 0 | grep -E "Algorithm version|Average execution time|Minimum execution time" > performance-output.txt 

for i in {1..4}
do
	echo "Running parallel version with $i threads"
	./find-communities-OpenMP ../graphs/cond-mat-2003.graph -f 2 -b 2 -a 1 -t $i | grep -E "Algorithm version|Number of threads:                |Average execution time|Minimum execution time" >> performance-output.txt
done
