# subset_mtx_graph.py
# Extracts a subset of nodes from a MatrixMarket (.mtx) file starting at an offset

import sys

if len(sys.argv) < 5:
    print("Usage: python subset_mtx_graph.py <input.mtx> <output.mtx> <num_nodes> <offset>")
    sys.exit(1)

in_file = sys.argv[1]
out_file = sys.argv[2]
limit = int(sys.argv[3])
offset = int(sys.argv[4])

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
                header_line = line  # preserve original counts
                continue
        u, v = map(int, line.strip().split())
        if offset <= u < offset + limit and offset <= v < offset + limit:
            # Normalize to start from 1 again
            edges.append((u - offset + 1, v - offset + 1))

with open(out_file, 'w') as f:
    f.write("%%MatrixMarket matrix coordinate pattern symmetric\n")
    f.write(header)
    f.write(f"{limit} {limit} {len(edges)}\n")
    for u, v in edges:
        f.write(f"{u} {v}\n")

print(f"Subset created: {out_file} with {limit} nodes starting from {offset}, total {len(edges)} edges")

