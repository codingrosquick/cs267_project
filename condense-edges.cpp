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

void dump_cluster_graph_in_METIS(int n_clusters, int n_cluster_edges, int *cluster_graph, std::string output_graph_file) {
    std::ofstream output_graph_fp;
    output_graph_fp.open( output_graph_file );
    output_graph_fp << n_clusters << " " << n_cluster_edges << "\n";

    // TODO: do the duplication thingy after talking to aravinth

    output_graph_fp.close();
}

void color_all_nodes(int n_nodes, list<bool>& node_colors_mapping, std::vector<int>& node_to_cluster_mapping, std::vector<bool>& cluster_to_color_mapping) {
    // TODO: parallelize this
    for (int u = 0; u < n_nodes; ++u) {
        int cluster = node_to_cluster_mapping[u];
        bool color = cluster_to_color_mapping[cluster];
        // If color is +1, we change the mapping.
        // If color is 0, the mapping stays unchanged.
        if (color) {
            node_colors_mapping[u] = true;
        }

    }        
}

int count_wrong_edges(int n_nodes, std::vector<std::vector<std::pair<int,int>>>& initial_graph, list<bool>& node_colors) {
    int frustration_count = 0;
    
    // TODO: parallelize this loop as well
    for (int u = 0; u < n_nodes; ++u) {
        bool current_node_color = node_colors[u];
        for (std::vector<std::pair<int,int>>::iterator it = u_nbrs.begin(); it != u_nbrs.end(); ++it) {
            other_color = node_colors[it.first];
            edge = it.second;
            // test for equality: (((other_color == current_node_color) && (edge == 1)) || ((other_color != current_node_color) && (edge == -1)))
            if (((other_color == current_node_color) && (edge == -1)) || ((other_color != current_node_color) && (edge == 1))) {
                ++frustration_count;
            }
        }
    }
    // probably needs a reduce to sum all of this up
    
    return frustration_count;
}

int main(int argc, char** argv) {
    int n_nodes, n_clusters, n_cluster_edges = 0;
    std::string cluster_file = "/global/homes/a/afrozalm/proj/dataset/wikiElec.triples.hipmcl";
    std::string initial_graph_file = "/global/homes/a/afrozalm/proj/dataset/cs267_project/wikiElec.orig";
    std::string output_graph_file = "/global/homes/a/afrozalm/proj/dataset/scotch-graph.txt";

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

    // TODO: parallel here
    #pragma openmp parallel for
    for (int u = 0; u < n_nodes; ++u) {
        int u_cluster = node_to_cluster_map[u];
        int *u_cluster_offset = cluster_graph + u_cluster * n_clusters;
        std::vector<std::pair<int,int>> u_nbrs = initial_graph[u];
        for (std::vector<std::pair<int,int>>::iterator it = u_nbrs.begin(); it != u_nbrs.end(); ++it) {
            int v = it->first, e = it->second;
            int v_cluster = node_to_cluster_map[v];
            if (u_cluster != v_cluster) {
                n_cluster_edges++;
                u_cluster_offset[v_cluster] += e;
            }
        }
    }

    dump_cluster_graph_in_METIS(n_clusters, n_cluster_edges, cluster_graph, output_graph_file);
    // return 0;


    // Suppose we have the cluster colors in a mapping std::vector<bool>
    // We put node colors into "node_colors" map
    // We can also log the edges to be changed
    std::vector<bool> cluster_to_color_mapping = [];

    color_all_nodes(n_nodes, node_colors_mapping, node_to_cluster_mapping, cluster_to_color_mapping);
    frustration_index = count_wrong_edges(n_nodes, initial_graph, node_colors);
    return frustration_index;
}
