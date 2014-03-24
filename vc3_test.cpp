#include <iostream>

#include "vc3.hpp"
#include "draw_network.hpp"

#include <set>

using namespace dda_simulator;
using namespace std;

// The real network.
Network net;

// The simulated network, with each node v doubled to nodes v and v + net.size().
Network simnet;

size_t end_round;
vector<pair<size_t, size_t>> pairs;

struct Listener : public EmptyListener<VC3Machine> {
	void send(
		size_t round,
		const vector<State>& states,
		const vector<vector<Message>>& msgs
	) {
		stringstream name;
		name << "vc3_out_" << round + 1;
		end_round = round + 1;
		
		auto msg_color = [](const BMMMachine::Message& msg) {
			if(msg == BMMMachine::Message::proposal) return "red";
			if(msg == BMMMachine::Message::accept) return "green";
			return "blue";
		};
		
		vector<tuple<size_t, size_t, string>> arrows;
		for(size_t v = 0; v < net.size(); ++v) {
			for(size_t ei = 0; ei < net[v].size(); ++ei) {
				if(msgs[v][ei].first != BMMMachine::Message::nothing) {
					arrows.emplace_back(v, ei, msg_color(msgs[v][ei].first));
				}
				if(msgs[v][ei].second != BMMMachine::Message::nothing) {
					arrows.emplace_back(v + net.size(), ei, msg_color(msgs[v][ei].second));
				}
			}
		}
		
		auto node_color = [](size_t v) {
			return v < net.size() ? "black" : "white";
		};
		
		auto edge_color = [&](size_t v1, size_t e1, size_t v2, size_t e2) {
			if(v1 < net.size()) {
				return states[v1].state2.match == e1 ? "red" : "black";
			} else {
				return states[v1 - net.size()].state1.match == e1 ? "red" : "black";
			}
		};
		
		auto node_label = [](size_t v) {
			std::stringstream ss;
			if(v < net.size()) {
				ss << v;
			} else {
				ss << v - net.size();
			}
			return ss.str();
		};
		
		drawNetwork(simnet, name.str(), arrows, node_color, edge_color, node_label, pairs);
	}
};

int main() {
	VC3Machine vc3;
	
	net = readNetwork();
	
	simnet.resize(2 * net.size());
	for(std::size_t v = 0; v < net.size(); ++v) {
		pairs.emplace_back(v, v + net.size());
		
		simnet[v] = net[v];
		simnet[v + net.size()] = net[v];
		
		for(Edge& e : simnet[v]) {
			e.dest += net.size();
		}
	}
	
	Listener listener;
	vector<VC3Machine::State> states =
		simulatePortNumberedDDA(net, vc3, vector<bool>(net.size()), listener);
	
	// Draw the final graph.
	stringstream name;
	name << "vc3_out_" << end_round + 1;
	
	auto node_color = [&](size_t v) {
		return states[v].isInVertexCover() ? "red" : "white";
	};
	
	drawNetwork(net, name.str(), vector<tuple<size_t, size_t, string>>(), node_color);
	
	return 0;
}
