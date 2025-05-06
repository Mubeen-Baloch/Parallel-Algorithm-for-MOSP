// parallel_update.cpp
/*
This file will:
- Initialize MPI
- Load local partition
- Call run_parallel_update()
- Finalize MPI
*/

#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <cstdlib>
#include <sstream> // Include this for std::istringstream

const int INF = std::numeric_limits<int>::max();
const int MAX_ITER = 100;
const int MAX_PARETO_SIZE = 100;

// Updated Edge structure to handle multiple weights
struct Edge {
    int target;
    std::vector<int> weights; // Vector of weights for multi-objective optimization
};

struct Vertex {
    int id;
    std::vector<Edge> edges;
    int distance;
    int predecessor;
};

struct ObjectiveVector {
    std::vector<int> objectives; // Vector of objectives (e.g., distance, cost)

    bool dominates(const ObjectiveVector& other) const {
        bool strictly_better = false;
        for (size_t i = 0; i < objectives.size(); ++i) {
            if (objectives[i] > other.objectives[i]) return false;
            if (objectives[i] < other.objectives[i]) strictly_better = true;
        }
        return strictly_better;
    }

    bool operator==(const ObjectiveVector& other) const {
        return objectives == other.objectives;
    }
};

using ParetoFront = std::vector<ObjectiveVector>;
std::unordered_map<int, Vertex> local_graph;
std::unordered_map<int, ParetoFront> pareto_sets;

void load_local_partition(const std::string& graph_file, const std::string& part_file, int world_rank, int source) {
    std::ifstream gfile(graph_file);
    std::ifstream pfile(part_file);

    if (!gfile.is_open() || !pfile.is_open()) {
        std::cerr << "Error: Unable to open graph or partition file.\n";
        MPI_Finalize();
        exit(1);
    }

    int n, m;
    gfile >> n >> m;

    std::vector<int> xadj(n + 1, 0); // Cumulative degree array
    std::vector<Edge> edges;         // List of edges with full weight vectors

    std::string line;
    int edge_count = 0;

    // Read adjacency list and weights
    std::cout << "Parsing graph file...\n";
    while (std::getline(gfile, line)) {
        // Skip empty lines
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos) {
            continue;
        }

        std::istringstream iss(line);
        int u;
        iss >> u;

        std::cout << "Vertex: " << u << " -> ";
        int v;
        while (iss >> v) {
            Edge e;
            e.target = v;

            std::vector<int> edge_weights;
            int weight;
            while (iss >> weight) {
                edge_weights.push_back(weight);
            }

            if (edge_weights.empty()) {
                edge_weights = {1, 1}; // Default weight vector
            }

            e.weights = edge_weights;
            edges.push_back(e);

            std::cout << v << " [";
            for (size_t i = 0; i < edge_weights.size(); ++i) {
                std::cout << edge_weights[i] << (i < edge_weights.size() - 1 ? ", " : "");
            }
            std::cout << "] ";
        }
        std::cout << "\n";

        // Update cumulative degree array
        xadj[u + 1]++;
        edge_count++;
    }

    // Compute cumulative degrees
    for (int i = 1; i <= n; ++i) {
        xadj[i] += xadj[i - 1];
    }

    // Read partition file
    std::vector<int> partitions(n);
    std::cout << "Parsing partition file...\n";
    for (int i = 0; i < n; ++i) {
        pfile >> partitions[i];
        std::cout << "Vertex " << i << " assigned to rank " << partitions[i] << "\n";
    }

    // Load local graph for this rank
    std::cout << "Loading local graph for Rank " << world_rank << "...\n";
    for (int u = 0; u < n; ++u) {
        if (partitions[u] == world_rank) {
            Vertex v;
            v.id = u;
            v.distance = INF;
            v.predecessor = -1;

            for (int j = xadj[u]; j < xadj[u + 1]; ++j) {
                int neighbor = edges[j].target;
                std::vector<int> weights = edges[j].weights;

                v.edges.push_back({neighbor, weights});
            }

            local_graph[u] = v;
            pareto_sets[u] = {};

            std::cout << "Rank " << world_rank << ": Loaded vertex " << u << " with "
                      << v.edges.size() << " edges\n";
        }
    }

    std::cout << "Rank " << world_rank << " loaded " << local_graph.size() << " vertices\n";
    if (local_graph.count(source)) {
        std::cout << "Rank " << world_rank << " owns the source vertex " << source << "\n";
        local_graph[source].distance = 0;
        pareto_sets[source].push_back({{0, 0}}); // Initialize Pareto front for source
    } else {
        std::cout << "Rank " << world_rank << " does NOT own the source vertex " << source << "\n";
    }
}

bool insert_into_pareto(ParetoFront& front, const ObjectiveVector& new_vec) {
    if (front.size() >= MAX_PARETO_SIZE) return false;
    for (const auto& v : front) {
        if (v == new_vec) return false;
    }
    for (auto it = front.begin(); it != front.end();) {
        if (new_vec.dominates(*it)) {
            it = front.erase(it);
        } else if (it->dominates(new_vec)) {
            return false;
        } else {
            ++it;
        }
    }
    front.push_back(new_vec);
    return true;
}

void parallel_mosp_update(int world_size, int world_rank) {
    bool updated = true;
    int iteration = 0;

    while (updated && iteration < MAX_ITER) {
        updated = false;
        int local_updates = 0;

        std::vector<int> keys;
        for (const auto& pair : local_graph) {
            keys.push_back(pair.first);
        }

        #pragma omp parallel for reduction(+:local_updates)
        for (int i = 0; i < keys.size(); ++i) {
            int key = keys[i];
            Vertex& v = local_graph[key];
            ParetoFront& current_front = pareto_sets[v.id];

            for (Edge& e : v.edges) {
                for (const ObjectiveVector& vec : current_front) {
                    std::vector<int> new_objectives(vec.objectives.size());
                    for (size_t i = 0; i < vec.objectives.size(); ++i) {
                        new_objectives[i] = vec.objectives[i] + e.weights[i];
                    }

                    ObjectiveVector new_vec = {new_objectives};

                    if (local_graph.count(e.target)) {
                        #pragma omp critical
                        {
                            if (insert_into_pareto(pareto_sets[e.target], new_vec)) {
                                std::cout << "Rank " << world_rank << ": Updated Pareto front for vertex "
                                          << e.target << " with (";
                                for (size_t i = 0; i < new_objectives.size(); ++i) {
                                    std::cout << new_objectives[i] << (i < new_objectives.size() - 1 ? ", " : "");
                                }
                                std::cout << ")\n";
                                updated = true;
                                local_updates++;
                            }
                        }
                    }
                }
            }
        }

        int global_updated;
        MPI_Allreduce(&updated, &global_updated, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
        updated = global_updated;

        int global_updates;
        MPI_Reduce(&local_updates, &global_updates, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (world_rank == 0) {
            std::cout << "[MOSP] Iteration " << iteration + 1 << " complete. Total updates = " << global_updates << "\n";
        }

        MPI_Barrier(MPI_COMM_WORLD);
        iteration++;
    }

    if (iteration >= MAX_ITER && world_rank == 0) {
        std::cout << "[MOSP] Terminated after reaching max iterations. Possible infinite loop detected.\n";
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if (argc < 3) {
        if (world_rank == 0)
            std::cerr << "Usage: mpirun -np <np> ./parallel_update <graph_file> <partition_file> [source_vertex]\n";
        MPI_Finalize();
        return 1;
    }

    std::string graph_file = argv[1];
    std::string partition_file = argv[2];
    int source_vertex = (argc >= 4) ? std::stoi(argv[3]) : 0;

    if (world_rank == 0)
        freopen(("results/output_rank0.txt"), "w", stdout);
    else
        freopen(("results/output_rank" + std::to_string(world_rank) + ".txt").c_str(), "w", stdout);

    load_local_partition(graph_file, partition_file, world_rank, source_vertex);

    double start_time = MPI_Wtime();
    parallel_mosp_update(world_size, world_rank);
    double end_time = MPI_Wtime();

    double local_exec_time = end_time - start_time;
    double max_exec_time;
    MPI_Reduce(&local_exec_time, &max_exec_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    int local_flops = 0;
    for (const auto& pair : pareto_sets) {
        local_flops += pair.second.size();
    }

    int total_flops;
    MPI_Reduce(&local_flops, &total_flops, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
        std::cout << "[Performance] Execution Time: " << max_exec_time << " seconds\n";
        std::cout << "[Performance] Total FLOPs: " << total_flops << "\n";
        std::cout << "[Performance] MFLOPS: " << (max_exec_time > 0 ? (total_flops / (1e6 * max_exec_time)) : 0) << " MFLOPS\n";
    }

    for (const auto& pair : pareto_sets) {
        std::cout << "Process " << world_rank << " - Vertex " << pair.first << ": ";
        for (const auto& vec : pair.second) {
            std::cout << "(";
            for (size_t i = 0; i < vec.objectives.size(); ++i) {
                std::cout << vec.objectives[i] << (i < vec.objectives.size() - 1 ? ", " : "");
            }
            std::cout << ") ";
        }
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
