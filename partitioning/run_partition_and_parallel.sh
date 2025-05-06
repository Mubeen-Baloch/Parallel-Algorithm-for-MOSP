#!/bin/bash

# Check arguments
if [ "$#" -ne 3 ]; then
    echo "Usage: ./run_partition_and_parallel.sh <graph_file> <num_partitions> <executable>"
    echo "Example: ./run_partition_and_parallel.sh datasets/roadNet_CA_formatted.txt 4 parallel_update"
    exit 1
fi

GRAPH_FILE=$1
NUM_PARTS=$2
EXECUTABLE=$3
PART_FILE="${GRAPH_FILE}.part"

# Partition using gpmetis
echo "[INFO] Partitioning graph into $NUM_PARTS parts..."
./metis_partition "$GRAPH_FILE" "$NUM_PARTS"
if [ $? -ne 0 ]; then
    echo "[ERROR] metis_partition failed. Ensure METIS is installed and graph file is valid."
    exit 1
fi

# Rename partition file to match input expectations
# (no need to rename, metis_partition already outputs the correct file)
echo "[INFO] Partition file saved as: $PART_FILE"

# Run the MPI program
echo "[INFO] Running MPI program with $NUM_PARTS processes..."
mpirun -np "$NUM_PARTS" "./$EXECUTABLE" "$GRAPH_FILE" "$PART_FILE"


#chmod +x partitioning/run_partition_and_parallel.sh

# Example run for 4 processes
# ./run_partition_and_parallel.sh datasets/roadNet_CA_formatted.txt 4 parallel_update

