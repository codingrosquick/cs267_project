        #include <chrono>
        #include <climits>
        #include <cmath>
        #include <cstring>
        #include <fstream>
        #include <iostream>
        #include <random>
        #include <sstream>
        #include <vector>
        #include <map>

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

        void unpack_cluster(std::string& line, std::vector<int>& list_of_vertices) {
            
            std::istringstream ss(line);
            std::string s;
            
            while (ss >> s) {
                //std::cout << s << "\n";
                list_of_vertices.push_back(stoi(s));
            }
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

        void populate_cluster_map(std::string file, std::map<int,int>& cluster_map) {
            std::ifstream cluster_fp(file);
            std::string line;
            int num_nodes = -1;
            bool first_read = false;
            int line_count = 0;

            if (cluster_fp.is_open()) {
                while ( getline(cluster_fp, line) ) {
                    if (!first_read) {
                        first_read = true;
                    } else {
                        line_count++;
                        std::vector<int> list_of_vertices;
                        unpack_cluster(line, list_of_vertices);
                        for(int i : list_of_vertices){
                            cluster_map.insert({i, line_count});
                            //std::cout<<"{"<<i<<","<<line_count<<"}"<<"\n";
                        }
                    }
                }
            }
            
        }

        void populate_part_map(std::string file, std::map<int,int>& part_map) {
            std::ifstream part_fp(file);
            std::string line;
            int num_nodes = -1;
            bool first_read = false;
            int line_count = 0;

            if (part_fp.is_open()) {
                while ( getline(part_fp, line) ) {
                        line_count++;
                        std::istringstream ss(line);
                        std::string s;
            
                        while (ss >> s) {
                            part_map.insert({line_count,stoi(s)});
                        }
                }
            }
            
        }


        int main(int argc, char** argv) {
            int n_nodes, n_clusters, min_edge = INT_MAX, max_edge = INT_MIN;
            int normal_edges = 0;
            // if (argc != 4) {
            //     std::cout << "Usage ./inflate <initial_graph_file.orig> <cluster_file.hipmcl> <partition_file.txt>\n";
            // }
            std::string partition_file = argv[3];
            std::string cluster_file = argv[2];
            std::string initial_graph_file = argv[1];
            std::map<int, int> cluster_map;
            std::map<int, int> cluster_coloring_map;
            std::vector<std::vector<std::pair<int,int>>> initial_graph; // u -> [ (v, w), ... ]

            populate_initial_graph(initial_graph_file, initial_graph);
            populate_cluster_map(cluster_file, cluster_map);
            populate_part_map(partition_file, cluster_coloring_map);
            int frustration_index = 0;
            //std::cout << initial_graph.size() << "\n";
            for(int i=0;i<initial_graph.size();i++){
                //std::cout << i << "\n";
                for(auto nested_list : initial_graph[i]){
                    int cluster_node_1 = cluster_map[i];
                    int cluster_node_2 = cluster_map[nested_list.first];

                    int edge_val = std::abs(cluster_coloring_map[cluster_node_1]-cluster_coloring_map[cluster_node_2]);
                    std::cout << i << "," << nested_list.first << "\n";
                    if(nested_list.second == 1){
                        if(edge_val != 0){
                            frustration_index++;
                        }
                    }   

                    else if(nested_list.second == -1){
                        if(edge_val == 0){
                            frustration_index++;
                        }
                    }
                }
            }
            
            

            n_nodes = initial_graph.size();

            std::cout << "Frustration index is " << frustration_index/2 << "\n";
            return 0;
        }
