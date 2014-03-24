#pragma once

#include "network.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace dda_simulator {

/// Default listener for simulatePortNumberedDDA.
template <typename Machine>
struct EmptyListener {
	typedef typename Machine::State State;
	typedef typename Machine::Message Message;
	
	void start(
		std::size_t,
		const std::vector<State>&
	) { }
	void send(
		std::size_t,
		const std::vector<State>&,
		const std::vector<std::vector<Message>>&
	) { }
	void receive(
		std::size_t,
		const std::vector<State>&,
		const std::vector<std::vector<Message>>&
	) { }
	
	static EmptyListener<Machine> instance;
};

template <typename Machine>
EmptyListener<Machine> EmptyListener<Machine>::instance;

/// Simulate port-numbered deterministic distributed algorithm in 'network'.
/// The algorithm is a state machine given by an object of type Machine with
/// member types
///  - State for the states of the algorithm.
///  - Input for the local inputs of the algorithm.
///  - Message for the messages sent in the algorithm.
/// and member functions
///  - init(deg, input) returns the initial state for node with degree 'deg'
///    and local input 'input' given in parameter 'inputs'.
///  - send(state, msgs) writes the messages sent to ports to a vector 'msgs'
///    indexed by port numbers 0..(degree - 1). Initially the vector is empty.
///  - receive(state, msgs) returns the next state when the messages received
///    from all ports is given in vector 'msgs', indexed by the ports.
///  - stopped(state) returns true iff a state is a stopped state.
///
/// If all nodes stop, the results are returned as a vector of the stopping
/// states indexed by the vertices.
///
/// If 'listener' is supplied, its methods are called as follows:
///  - start(round, states) is called in the beginning of every round
///    with the number of the round and the state vector.
///  - send(round, states, msgs) is called after start with the
///    vector of sent messages by node and port number.
///  - receive(round, states, msgs) is called after send with the
///    vector of messages to be received by node and port number.
template <typename Machine, typename Listener = EmptyListener<Machine>>
std::vector<typename Machine::State> simulatePortNumberedDDA(
	const Network& network,
	const Machine& machine,
	const std::vector<typename Machine::Input>& inputs,
	Listener& listener = EmptyListener<Machine>::instance
) {
	typedef typename Machine::State State;
	typedef typename Machine::Message Message;
	
	std::size_t n = network.size();
	if(n != inputs.size()) {
		throw std::runtime_error(
			"simulatePortNumberedDDA: Wrong number of inputs given."
		);
	}
	
	std::vector<State> states;
	
	// Initialize all states from init function.
	for(std::size_t i = 0; i < n; ++i) {
		states.emplace_back(machine.init(network[i].size(), inputs[i]));
	}
	
	std::vector<std::vector<Message>> msgs(n);
	
	std::size_t round = 0;
	while(true) {
		listener.start(round, (const std::vector<State>&)states);
		
		// If all states are stopped, we are done.
		bool done = true;
		for(const State& state : states) {
			if(!machine.stopped(state)) {
				done = false;
				break;
			}
		}
		if(done) break;
		
		// Send all messages.
		for(std::size_t i = 0; i < n; ++i) {
			msgs[i].clear();
			machine.send(states[i], msgs[i]);
			if(msgs[i].size() != network[i].size()) {
				throw std::runtime_error(
					"simulatePortNumberedDDA: "
					"'send' function wrote wrong number of messages."
				);
			}
		}
		
		listener.send(
			round,
			(const std::vector<State>&)states,
			(const std::vector<std::vector<Message>>&)msgs
		);
		
		// Swap messages for each port.
		for(std::size_t i = 0; i < n; ++i) {
			for(std::size_t ei = 0; ei < network[i].size(); ++ei) {
				Edge edge = network[i][ei];
				
				// Swap only once.
				if(std::make_pair(i, ei) < std::make_pair(edge.dest, edge.back_index)) {
					continue;
				}
				
				std::swap(msgs[i][ei], msgs[edge.dest][edge.back_index]);
			}
		}
		
		listener.receive(
			round,
			(const std::vector<State>&)states,
			(const std::vector<std::vector<Message>>&)msgs
		);
		
		// Receive all messages and update states.
		for(std::size_t i = 0; i < n; ++i) {
			states[i] = machine.receive(states[i], msgs[i]);
		}
		
		++round;
	}
	
	return states;
}

}
