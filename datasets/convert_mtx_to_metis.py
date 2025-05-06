# convert_mtx_to_metis.py
# Converts MatrixMarket-format graph (roadNet-CA.mtx) to METIS CSR format (1-based indexing)

from collections import defaultdict
import sys

if len(sys.argv) < 3:
    print("Usage: python convert_mtx_to_metis.py <input.mtx> <output.txt>")
    sys.exit(1)

input_file = sys.argv[1]
output_file = sys.argv[2]

adj = defaultdict(set)
num_vertices = 0
line_count = 0

with open(input_file, 'r') as f:
    for line in f:
        line_count += 1
        if line.startswith('%') or line.strip() == '':
            continue
        if num_vertices == 0:
            parts = line.strip().split()
            if len(parts) != 3:
                print(f"Invalid header at line {line_count}")
                sys.exit(1)
            num_vertices = int(parts[0])
            for i in range(1, num_vertices + 1):
                adj[i] = set()  # 1-based
            continue

        try:
            u, v = map(int, line.strip().split())
            if u == v or u < 1 or v < 1 or u > num_vertices or v > num_vertices:
                print(f"Skipping invalid edge ({u}, {v}) at line {line_count}")
                continue
            adj[u].add(v)
            adj[v].add(u)
        except ValueError:
            print(f"Invalid edge format at line {line_count}: {line.strip()}")
            continue

# METIS expects: first line = num_vertices num_edges
# Each line i (1-based) = adjacency list of vertex i (also 1-based)
num_edges = sum(len(neighs) for neighs in adj.values()) // 2

with open(output_file, 'w') as f:
    f.write(f"{num_vertices} {num_edges}\n")
    for i in range(1, num_vertices + 1):
        neighbors = sorted(adj[i])
        if neighbors:
            f.write(' '.join(map(str, neighbors)) + '\n')
        else:
            f.write('\n')

print(f"Converted {input_file} -> {output_file} [METIS format, 1-based indexing]")

