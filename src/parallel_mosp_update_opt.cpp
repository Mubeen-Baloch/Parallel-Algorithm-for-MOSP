#include <mpi.h>
#include <omp.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <climits>
#include <queue>
#include <unordered_map>
#include <algorithm>

using namespace std;

const int INF = INT_MAX;

void read_graph(const string &filename, vector<vector<int>> &adj_list, int &num_nodes) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        exit(1);
    }

    int edges;
    infile >> num_nodes >> edges;
    adj_list.resize(num_nodes + 1);  // 1-based indexing safety

    string line;
    getline(infile, line); // consume the rest of the first line

    int node = 1;
    while (getline(infile, line)) {
        istringstream iss(line);
        int neighbor;
        while (iss >> neighbor) {
            if (neighbor >= 1 && neighbor <= num_nodes)
                adj_list[node].push_back(neighbor);
        }
        node++;
    }
    infile.close();
}

void read_partitions(const string &filename, vector<int> &partitions, int num_nodes) {
    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error opening partition file: " << filename << endl;
        exit(1);
    }

    partitions.resize(num_nodes + 1);  // 1-based safety
    int node = 1, part;
    while (infile >> part && node <= num_nodes) {
        partitions[node] = part;
        node++;
    }
    infile.close();
}

void mosp(const vector<vector<int>> &local_adj, int source, vector<int> &dist) {
    int n = local_adj.size();
    dist.assign(n, INF);
    dist[source] = 0;
    queue<int> q;
    q.push(source);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : local_adj[u]) {
            if (dist[v] == INF) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 4) {
        if (rank == 0)
            cerr << "Usage: " << argv[0] << " <graph_file> <partition_file> <source_node>" << endl;
        MPI_Finalize();
        return 1;
    }

    string graph_file = argv[1];
    string partition_file = argv[2];
    int source_node = stoi(argv[3]);

    vector<vector<int>> adj_list;
    int num_nodes;

    if (rank == 0) {
        read_graph(graph_file, adj_list, num_nodes);
    }

    MPI_Bcast(&num_nodes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank != 0) {
        adj_list.resize(num_nodes + 1);
    }

    for (int i = 1; i <= num_nodes; ++i) {
        int degree = adj_list[i].size();
        MPI_Bcast(&degree, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (rank != 0) {
            adj_list[i].resize(degree);
        }
        MPI_Bcast(adj_list[i].data(), degree, MPI_INT, 0, MPI_COMM_WORLD);
    }

    vector<int> partitions;
    if (rank == 0) {
        read_partitions(partition_file, partitions, num_nodes);
    }

    if (rank != 0) {
        partitions.resize(num_nodes + 1);
    }

    MPI_Bcast(partitions.data(), num_nodes + 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<vector<int>> local_adj(num_nodes + 1);

    for (int i = 1; i <= num_nodes; ++i) {
        if (partitions[i] == rank) {
            local_adj[i] = adj_list[i];
        }
    }

    vector<int> dist;
    if (partitions[source_node] == rank) {
        mosp(local_adj, source_node, dist);
        for (int i = 1; i < dist.size(); ++i) {
            if (dist[i] != INF)
                cout << "Node " << i << " Distance: " << dist[i] << endl;
        }
    }

    MPI_Finalize();
    return 0;
}

