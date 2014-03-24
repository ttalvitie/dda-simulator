#pragma once

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace dda_simulator {

/// Edge of a network.
struct Edge {
	/// Index of the destination vertex.
	std::size_t dest;
	
	/// Index of this edge in the adjacency list in vertex 'dest'.
	std::size_t back_index;
};

/// Networks are given as vectors containing the adjacency list as vector
/// for each vertex.
typedef std::vector<std::vector<Edge>> Network;

/// Add edge (a, b) to 'network'. Assumes that the network has large enough size
/// for both parameters.
void addEdge(Network& network, std::size_t a, std::size_t b) {
	std::size_t a_ind = network[a].size();
	network[a].emplace_back();
	std::size_t b_ind = network[b].size();
	network[b].emplace_back();
	
	network[a][a_ind].dest = b;
	network[a][a_ind].back_index = b_ind;
	
	network[b][b_ind].dest = a;
	network[b][b_ind].back_index = a_ind;
}

/// Read a simple network by asking the adjacency lists from standard input.
Network readNetwork() {
	Network ret;
	
	std::cout << "Number of nodes: ";
	std::size_t n;
	std::cin >> n;
	while(std::cin.get() != '\n') { }
	ret.resize(n);
	
	std::cout << "The nodes are numbered 0.." << n - 1 << "\nGive the adjacency lists as space separated lists of numbers.\n";
	for(std::size_t i = 0; i < n; ++i) {
		std::cout << i << ": ";
		std::string line;
		std::getline(std::cin, line);
		line.push_back('\n');
		std::stringstream ss(line);
		
		while(true) {
			std::size_t j;
			ss >> j;
			if(!ss.good()) break;
			if(j >= n) {
				throw std::runtime_error("readNetwork: Node index out of bounds.");
			}
			if(i == j) {
				throw std::runtime_error("readNetwork: Loop created.");
			}
			
			ret[i].push_back(Edge{j, 0});
		}
	}
	
	for(std::size_t i = 0; i < n; ++i) {
		for(Edge& e : ret[i]) {
			bool found = false;
			for(std::size_t k = 0; k < ret[e.dest].size(); ++k) {
				if(ret[e.dest][k].dest == i) {
					if(found) {
						std::stringstream err;
						err << "readNetwork: Multiple edges " << i << " - " << e.dest << ".";
						throw std::runtime_error(err.str());
					}
					found = true;
					e.back_index = k;
				}
			}
			if(!found) {
				std::stringstream err;
				err << "readNetwork: No backward edge for " << i << " - " << e.dest << ".";
				throw std::runtime_error(err.str());
			}
		}
	}
	
	return ret;
}

}
