import random

def generate_metis_graph(num_nodes=1000, avg_degree=4, filename="datasets/dummy_1000_nodes.txt"):
    adjacency = {i: set() for i in range(num_nodes)}
    
    for node in range(num_nodes):
        while len(adjacency[node]) < avg_degree:
            neighbor = random.randint(0, num_nodes - 1)
            if neighbor != node:
                adjacency[node].add(neighbor)
                adjacency[neighbor].add(node)

    with open(filename, "w") as f:
        f.write(f"{num_nodes} {sum(len(neighs) for neighs in adjacency.values()) // 2}\n")
        for i in range(num_nodes):
            neighbors = sorted([n + 1 for n in adjacency[i]])  # METIS is 1-based
            f.write(" ".join(map(str, neighbors)) + "\n")

    print(f"Dummy METIS graph written to {filename}")

generate_metis_graph()

