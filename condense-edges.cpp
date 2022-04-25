/**
 * @file condense-edges.cpp
 * @author Afroz Alam (afrozalm@berkeley.edu)
 * @brief
 * @version 0.1
 * @date 2022-04-24
 *
 * @copyright Copyright (c) 2022
 *
 */

/*  node -> group hashmap association
    node-> node edge wt hashmap
    group -> group edge wt hashmap
 */

#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <sstream>

void unpack(std::string& line, int& u, int& v, int& e) {
    int temp[3], idx = 0;
    std::istringstream ss(line);
    std::string s;
    while (getline(ss, s, '\t')) {
        temp[idx++] = stoi(s);
    }
    u = temp[0];
    v = temp[1];
    e = temp[2];
}

void populate_initial_graph(std::string file, std::vector<std::vector<std::pair<int,int>>>& initial_graph) {
    std::ifstream initial_graph_fp(file);
    std::string line;
    int num_nodes = -1;
    bool first_read = false;

    if (initial_graph_fp.is_open()) {
        while ( getline(initial_graph_fp, line) ) {
            if (!first_read) {
                first_read = true;
                num_nodes = stoi(line);
                initial_graph = std::vector<std::vector<std::pair<int,int>>>(num_nodes);
            } else {
                int u, v, e;
                unpack(line, u, v, e);
                initial_graph[u].push_back({v, e});
            }
        }
    }
    std::cout << "num nodes = " << num_nodes << "\n";
}

int main(int argc, char** argv) {
    // in files 1. clusters 2. initial edges
    // outfile clustered weighted graph
    std::string cluster_file = "path/to/cluster/file.hipmcl";
    std::string initial_graph_file = "/global/homes/a/afrozalm/proj/dataset/cs267_project/wikiElec.orig";
    std::string output_graph_file = "path/to/final/graph.txt";

    std::vector<std::vector<std::pair<int,int>>> initial_graph; // u -> [ (v, w), ... ]
    std::vector<std::vector<int>> clusters; // [[u1, u2, u3, ...], [v1, v2, ..]]

    populate_initial_graph(initial_graph_file, initial_graph);

    return 0;
}
