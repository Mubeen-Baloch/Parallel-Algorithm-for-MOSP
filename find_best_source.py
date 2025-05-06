# find_best_source.py
# Find the vertex with highest degree from METIS-formatted file

import sys

if len(sys.argv) < 2:
    print("Usage: python3 find_best_source.py <metis_file>")
    exit(1)

path = sys.argv[1]

max_degree = -1
max_vertex = -1

with open(path) as f:
    header = f.readline()
    for idx, line in enumerate(f, 1):
        degree = len(line.strip().split())
        if degree > max_degree:
            max_degree = degree
            max_vertex = idx

print(f"Best source candidate: Vertex {max_vertex} (Degree: {max_degree})")

