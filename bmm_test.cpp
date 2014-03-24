#include <iostream>

#include "bmm.hpp"
#include "draw_network.hpp"

#include <set>

using namespace dda_simulator;
using namespace std;

Network net;
vector<bool> coloring;

size_t end_round;

struct Listener : public EmptyListener<BMMMachine> {
	void send(
		size_t round,
		const vector<State>& states,
		const vector<vector<Message>>& msgs
	) {
		stringstream name;
		name << "bmm_out_" << round + 1;
		end_round = round + 1;
		
		vector<tuple<size_t, size_t, string>> arrows;
		for(size_t v = 0; v < net.size(); ++v) {
			for(size_t ei = 0; ei < net[v].size(); ++ei) {
				if(msgs[v][ei] != Message::nothing) {
					string color;
					if(msgs[v][ei] == Message::proposal) color = "red";
					if(msgs[v][ei] == Message::accept) color = "green";
					if(msgs[v][ei] == Message::matched) color = "blue";
					arrows.emplace_back(v, ei, color);
				}
			}
		}
		
		auto node_color = [](size_t v) {
			return coloring[v] ? "black" : "white";
		};
		
		auto edge_color = [&](size_t v1, size_t e1, size_t v2, size_t e2) {
			return states[v1].match == e1 ? "red" : "black";
		};
		
		drawNetwork(net, name.str(), arrows, node_color, edge_color);
	}
};

int main() {
	BMMMachine bmm;
	
	net = readNetwork();
	
	coloring.resize(net.size());
	cout << "Input node colors (0 or nonzero):\n";
	for(size_t i = 0; i < net.size(); ++i) {
		cout << i << ": ";
		int x;
		cin >> x;
		coloring[i] = (bool)x;
	}
	
	Listener listener;
	vector<BMMMachine::State> states =
		simulatePortNumberedDDA(net, bmm, coloring, listener);
	
	// Draw the final graph.
	vector<vector<BMMMachine::Message>> fakemsgs(net.size());
	for(size_t v = 0; v < net.size(); ++v) {
		fakemsgs[v].resize(net[v].size(), BMMMachine::Message::nothing);
	}
	listener.send(end_round, states, fakemsgs);
	
	return 0;
}
