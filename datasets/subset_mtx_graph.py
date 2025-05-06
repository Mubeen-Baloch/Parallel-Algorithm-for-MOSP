# subset_mtx_graph.py
# Extracts the first N nodes and their edges from a large MatrixMarket (.mtx) file

import sys

if len(sys.argv) < 4:
    print("Usage: python subset_mtx_graph.py <input.mtx> <output.mtx> <num_nodes>")
    sys.exit(1)

in_file = sys.argv[1]
out_file = sys.argv[2]
limit = int(sys.argv[3])

header = ""
edges = []

with open(in_file, 'r') as f:
    for line in f:
        if line.startswith('%'):
            header += line
            continue
        if len(edges) == 0:
            parts = line.strip().split()
            if len(parts) == 3:
                full_nodes = int(parts[0])
                header_line = line  # preserve original counts
                continue
        u, v = map(int, line.strip().split())
        if u <= limit and v <= limit:
            edges.append((u, v))

with open(out_file, 'w') as f:
    f.write("%%MatrixMarket matrix coordinate pattern symmetric\n")
    f.write(header)
    f.write(f"{limit} {limit} {len(edges)}\n")
    for u, v in edges:
        f.write(f"{u} {v}\n")

print(f"Subset created: {out_file} with {limit} nodes and {len(edges)} edges")

