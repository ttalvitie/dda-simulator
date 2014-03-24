#pragma once

#include "network.hpp"

#include <fstream>
#include <functional>
#include <cstdlib>
#include <tuple>

namespace dda_simulator {

/// Draw 'network' to 'name'.pdf and 'name'.png.
/// The colors/labels of nodes and edges are given by functions
///  - node_color which takes node index and returns the color string.
///  - edge_color which takes (node1, node1_edge, node2, node2_edge) and
///    returns the color string.
///  - node_label which takes node index and returns the label string.
/// Additionally, the additional arrows in the network are given as tuples
/// (node, node_edge, color).
/// Nodes may be paired together by specifying list of pairs.
void drawNetwork(
	const Network& network,
	const std::string& name,
	const std::vector<std::tuple<std::size_t, std::size_t, std::string>>& arrows =
		std::vector<std::tuple<std::size_t, std::size_t, std::string>>(),
	std::function<std::string(std::size_t)> node_color =
		[](std::size_t) { return "white"; },
	std::function<std::string(std::size_t, std::size_t, std::size_t, std::size_t)> edge_color =
		[](std::size_t, std::size_t, std::size_t, std::size_t) { return "black"; },
	std::function<std::string(std::size_t)> node_label =
		[](std::size_t v) {
			std::stringstream ss;
			ss << v;
			return ss.str();
		},
	 const std::vector<std::pair<std::size_t, std::size_t>>& pairs =
		std::vector<std::pair<std::size_t, std::size_t>>()
) {
	std::string dotname = name;
	dotname.append(".dot");
	std::ofstream dotfp(dotname);
	dotfp << "digraph {\n";
	
	// Draw nodes.
	for(std::size_t v = 0; v < network.size(); ++v) {
		std::string color = node_color(v);
		std::string fontcolor = (color == "black") ? "white" : "black";
		
		dotfp << "  " << v << " [";
		dotfp << "style=filled";
		dotfp << ", fillcolor=" << color;
		dotfp << ", fontcolor=" << fontcolor;
		dotfp << ", label=\"" << node_label(v) << "\"";
		dotfp << ", shape=circle";
		dotfp << "]\n";
	}
	
	for(std::pair<std::size_t, std::size_t> p : pairs) {
		dotfp << p.first << " -> " << p.second << " [dir=none, weight=50, penwidth=3]\n";
	}
	
	// Draw edges.
	for(std::size_t v = 0; v < network.size(); ++v) {
		for(std::size_t ei = 0; ei < network[v].size(); ++ei) {
			Edge e = network[v][ei];
			if(e.dest > v) continue;
			
			std::string color = edge_color(v, ei, e.dest, e.back_index);
			
			dotfp << "  " << v << " -> " << e.dest << " [";
			dotfp << "taillabel=" << ei + 1;
			dotfp << ", headlabel=" << e.back_index + 1;
			dotfp << ", color=" << color;
			dotfp << ", dir=none";
			dotfp << ", len=2";
			dotfp << "]\n";
		}
	}
	
	// Draw arrows.
	for(const std::tuple<std::size_t, std::size_t, std::string>& arrow : arrows) {
		std::size_t v, ei;
		std::string color;
		std::tie(v, ei, color) = arrow;
		
		dotfp << "  " << v << " -> " << network[v][ei].dest << " [";
		dotfp << "color=" << color;
		dotfp << ", constraint=false";
		dotfp << "]\n";
	}
	
	dotfp << "}\n";
	dotfp.close();
	
	std::string pngname = name;
	pngname.append(".png");
	std::string pdfname = name;
	pdfname.append(".pdf");
	
	std::stringstream cmdpng;
	cmdpng << "neato -Tpng " << dotname << " > " << pngname;
	std::stringstream cmdpdf;
	cmdpdf << "neato -Tpdf " << dotname << " > " << pdfname;
	int res = system(cmdpng.str().c_str());
	if(res != 0) throw std::runtime_error("Calling neato failed.");
	res = system(cmdpdf.str().c_str());
	if(res != 0) throw std::runtime_error("Calling neato failed.");
	std::cout << "Wrote network to ";
	std::cout << dotname << ", " << pngname << " and " << pdfname << "\n";
}

}
