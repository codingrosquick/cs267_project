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
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

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

void populate_clusters(std::string& file, std::vector<std::vector<int>>& clusters, std::vector<int>& node_to_cluster_mapping) {
    std::ifstream cluster_fp(file);
    std::string line;
    int cluster_idx = 0;

    if (cluster_fp.is_open()) {
        while ( getline( cluster_fp, line ) ) {
            if ( line.size() == 1 )
                continue;
            std::istringstream ss(line);
            std::string s;
            std::vector<int> current_cluster;
            while ( getline(ss, s, ' ') ) {
                int node = stoi(s);
                current_cluster.push_back( node );
                node_to_cluster_mapping[node] = cluster_idx;
            }
            cluster_idx++;
            clusters.push_back( current_cluster );
        }
    }
    std::cout << "found " << clusters.size() << " clusters\n";
}

void dump_cluster_graph_in_METIS(int n_clusters, int min_edge, int max_edge, int *cluster_graph, std::string output_graph_file) {
    int total_edges = 0;
    std::string out_buffer;

    // TODO: do the duplication thingy after talking to aravinth
    // cal the minimimum, use that as an offset, then do e^(c* (wt+offset))
    for (int u = 0; u < n_clusters; ++u) {
        int *u_adjacency = cluster_graph + u * n_clusters;
        for (int v = 0; v < n_clusters; ++v) {
            int e = u_adjacency[v];
            if ( e < 0 ) {
                total_edges++;
                out_buffer.append(std::to_string(v));
                out_buffer.push_back(' ');
                // output_graph_fp << v << " ";
            } else if (e > 0) {
                total_edges += 2;
                out_buffer.append(std::to_string(v));
                out_buffer.push_back(' ');
                out_buffer.append(std::to_string(v));
                out_buffer.push_back(' ');
            }
        }
        out_buffer.push_back('\n');
    }

    std::ofstream output_graph_fp;
    output_graph_fp.open( output_graph_file );
    output_graph_fp << n_clusters << " " << total_edges << "\n";
    output_graph_fp << out_buffer;
    output_graph_fp.close();
    std::cout << "with total edges: " << total_edges << "\n";
}

// void color_all_nodes(int n_nodes, list<bool>& node_colors_mapping, std::vector<int>& node_to_cluster_mapping, std::vector<bool>& cluster_to_color_mapping) {
//     // TODO: parallelize this
//     for (int u = 0; u < n_nodes; ++u) {
//         int cluster = node_to_cluster_mapping[u];
//         bool color = cluster_to_color_mapping[cluster];
//         // If color is +1, we change the mapping.
//         // If color is 0, the mapping stays unchanged.
//         if (color) {
//             node_colors_mapping[u] = true;
//         }

//     }
// }

// int count_wrong_edges(int n_nodes, std::vector<std::vector<std::pair<int,int>>>& initial_graph, list<bool>& node_colors) {
//     int frustration_count = 0;

//     // TODO: parallelize this loop as well
//     for (int u = 0; u < n_nodes; ++u) {
//         bool current_node_color = node_colors[u];
//         for (std::vector<std::pair<int,int>>::iterator it = u_nbrs.begin(); it != u_nbrs.end(); ++it) {
//             other_color = node_colors[it.first];
//             edge = it.second;
//             // test for equality: (((other_color == current_node_color) && (edge == 1)) || ((other_color != current_node_color) && (edge == -1)))
//             if (((other_color == current_node_color) && (edge == -1)) || ((other_color != current_node_color) && (edge == 1))) {
//                 ++frustration_count;
//             }
//         }
//     }
//     // probably needs a reduce to sum all of this up

//     return frustration_count;
// }

int main(int argc, char** argv) {
    int n_nodes, n_clusters, min_edge = INT_MAX, max_edge = INT_MIN;
    int normal_edges = 0;
    if (argc != 4) {
        std::cout << "Usage ./condense <cluster_file.hipmcl> <initial_graph_file.orig> <output_graph_file.txt>\n";
    }
    std::string cluster_file = argv[1];
    std::string initial_graph_file = argv[2];
    std::string output_graph_file = argv[3];

    std::vector<std::vector<std::pair<int,int>>> initial_graph; // u -> [ (v, w), ... ]
    std::vector<std::vector<int>> clusters; // [[u1, u2, u3, ...], [v1, v2, ..]]
    std::vector<int> node_to_cluster_map;

    populate_initial_graph(initial_graph_file, initial_graph);

    n_nodes = initial_graph.size();
    node_to_cluster_map = std::vector<int>(n_nodes);

    bool node_colors_mapping[n_nodes] = { false }; // Used for coloring the nodes

    populate_clusters(cluster_file, clusters, node_to_cluster_map);

    n_clusters = clusters.size();
    int *cluster_graph = (int*) malloc( n_clusters * n_clusters * sizeof(int) );
    memset(cluster_graph, 0, n_clusters * n_clusters * sizeof(int));

    #pragma omp parallel for
    for (int u = 0; u < n_nodes; ++u) {
        int u_cluster = node_to_cluster_map[u];
        int *u_cluster_offset = cluster_graph + u_cluster * n_clusters;
        std::vector<std::pair<int,int>> u_nbrs = initial_graph[u];
        for (std::vector<std::pair<int,int>>::iterator it = u_nbrs.begin(); it != u_nbrs.end(); ++it) {
            int v = it->first, e = it->second;
            int v_cluster = node_to_cluster_map[v];
            if (u_cluster != v_cluster) {
                int empty = (u_cluster_offset[v_cluster] == 0);
                normal_edges += empty;
                u_cluster_offset[v_cluster] += e;
                min_edge = min(u_cluster_offset[v_cluster], min_edge);
                max_edge = max(u_cluster_offset[v_cluster], max_edge);
            }
        }
    }
    std::cout << "normal edge is " << normal_edges << "\n";
    dump_cluster_graph_in_METIS(n_clusters, min_edge, max_edge, cluster_graph, output_graph_file);
    return 0;
}
