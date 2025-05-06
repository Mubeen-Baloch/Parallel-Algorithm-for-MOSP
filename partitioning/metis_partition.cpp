#include <metis.h>
#include <iostream>
#include <vector>
#include <fstream>

void readGraph(const std::string& filename, std::vector<idx_t>& xadj, std::vector<idx_t>& adjncy) {
    std::ifstream infile(filename);
    idx_t n, m;
    infile >> n >> m;

    xadj.resize(n + 1);
    adjncy.resize(2 * m);

    for (idx_t i = 0; i <= n; ++i) {
        infile >> xadj[i];
    }

    for (idx_t i = 0; i < 2 * m; ++i) {
        infile >> adjncy[i];
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <graph_file> <num_partitions>" << std::endl;
        return 1;
    }

    std::string graph_file = argv[1];
    idx_t num_parts = std::stoi(argv[2]);

    std::vector<idx_t> xadj;
    std::vector<idx_t> adjncy;

    readGraph(graph_file, xadj, adjncy);

    idx_t nVertices = xadj.size() - 1;
    std::vector<idx_t> part(nVertices); // Partition vector
    idx_t objval;

    // Call METIS to partition the graph
    idx_t ncon = 1; // Number of balancing constraints (for now, 1)

METIS_PartGraphKway(
    &nVertices,      // Number of vertices
    &ncon,           // Number of constraints (1 usually)
    xadj.data(),     // xadj
    adjncy.data(),   // adjncy
    NULL,            // Vertex weights (NULL = uniform)
    NULL,            // Vertex sizes (NULL = default)
    NULL,            // Edge weights (NULL = uniform)
    &num_parts,      // Number of partitions
    NULL,            // Target partition weights (NULL = equal partitions)
    NULL,            // Allowed imbalance tolerance (NULL = default 1.03)
    NULL,            // Options array (NULL = defaults)
    &objval,         // Output: edge cuts
    part.data()      // Output: partition vector
);


    // Print partitions
    std::cout << "Vertex partitions:" << std::endl;
    for (idx_t i = 0; i < nVertices; ++i) {
        std::cout << "Vertex " << i << " -> Part " << part[i] << std::endl;
    }

    return 0;
}
//mpic++ -fopenmp partitioning/metis_partition.cpp -lmetis -o metis_partition
//./metis_partition datasets/small_graph.txt 2
