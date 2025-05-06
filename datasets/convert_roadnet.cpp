// convert_roadnet.cpp (Matrix Market version)
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./convert_roadnet <input_roadNet-CA.mtx> <output_graph.txt>\n";
        return 1;
    }

    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open input file." << std::endl;
        return 1;
    }

    std::string line;
    std::unordered_map<int, std::set<int>> adj;
    int max_node = 0;
    bool header_skipped = false;

    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '%') continue;

        std::istringstream iss(line);
        int u, v;

        if (!header_skipped) {
            // This is the "nRows nCols nEdges" line
            header_skipped = true;
            continue;
        }

        if (!(iss >> u >> v)) continue;

        u--; v--; // Matrix Market format is 1-indexed

        adj[u].insert(v);
        adj[v].insert(u); // undirected

        max_node = std::max({max_node, u, v});
    }

    infile.close();

    int num_nodes = max_node + 1;
    std::vector<int> xadj(num_nodes + 1, 0);
    std::vector<int> adjncy;

    for (int i = 0; i < num_nodes; ++i) {
        if (adj.count(i)) {
            xadj[i + 1] = xadj[i] + adj[i].size();
            adjncy.insert(adjncy.end(), adj[i].begin(), adj[i].end());
        } else {
            xadj[i + 1] = xadj[i];
        }
    }

    int num_edges = adjncy.size() / 2; // undirected

    outfile << num_nodes << " " << num_edges << "\n";
    for (int i = 0; i <= num_nodes; ++i) {
        outfile << xadj[i] << " ";
    }
    outfile << "\n";
    for (int i = 0; i < adjncy.size(); ++i) {
        outfile << adjncy[i] << " ";
    }
    outfile << "\n";

    outfile.close();
    std::cout << "Converted Matrix Market (.mtx) graph to CSR format: " << argv[2] << std::endl;
    return 0;
}

//g++ datasets/convert_roadnet.cpp -o convert_roadnet
//./convert_roadnet datasets/roadNet-CA/roadNet-CA.mtx datasets/roadNet_CA_formatted.txt
