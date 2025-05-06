# subset_bfs_mtx.py
# Extract a BFS-connected subgraph from a .mtx file

import sys
from collections import defaultdict, deque

if len(sys.argv) < 4:
    print("Usage: python3 subset_bfs_mtx.py <input.mtx> <output.mtx> <num_nodes> [source_node=1]")
    sys.exit(1)

input_path = sys.argv[1]
output_path = sys.argv[2]
num_nodes = int(sys.argv[3])
source_node = int(sys.argv[4]) if len(sys.argv) > 4 else 1

adj = defaultdict(set)
reading_edges = False

with open(input_path) as f:
    for line in f:
        if line.startswith('%'):
            continue
        if not reading_edges:
            n, _, _ = map(int, line.strip().split())
            reading_edges = True
            continue

        u, v = map(int, line.strip().split())
        adj[u].add(v)
        adj[v].add(u)

visited = set()
queue = deque([source_node])
visited.add(source_node)

while queue and len(visited) < num_nodes:
    u = queue.popleft()
    for v in adj[u]:
        if v not in visited:
            visited.add(v)
            queue.append(v)

used_nodes = sorted(visited)
node_map = {node: i + 1 for i, node in enumerate(used_nodes)}

edges = set()
for u in used_nodes:
    for v in adj[u]:
        if v in visited:
            a, b = node_map[u], node_map[v]
            if a < b:
                edges.add((a, b))

with open(output_path, 'w') as f:
    f.write(f"{len(used_nodes)} {len(edges)}\n")
    for a, b in sorted(edges):
        f.write(f"{a} {b}\n")

print(f"✔ Extracted {len(used_nodes)} nodes, {len(edges)} edges from BFS starting at node {source_node}")

#python3 datasets/subset_bfs_mtx.py datasets/roadNet-CA/roadNet-CA.mtx datasets/roadNet_CA_bfs_subset.mtx 10000 1

