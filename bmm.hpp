#pragma once

#include "simulator.hpp"

#include <climits>
#include <set>

namespace dda_simulator {

/// Port numbered BMM machine that finds a maximal matching in a 2-colored
/// simple graph. The 2-coloring should be given as input with color names
/// false for white and true for black.
/// The algorithm is described in
///   Jukka Suomela: A Course on Deterministic Distributed Algorithms
///   http://users.ics.aalto.fi/suomela/dda/
/// In the end, the matching may be read from the 'match' field of the states.
struct BMMMachine {
	struct State {
		bool black;
		std::size_t deg;
		std::size_t round;
		bool running;
		
		/// If matched, the port number of the matched vertex, otherwise SIZE_MAX.
		std::size_t match;
		
		/// For black nodes, the set M, which is the set of proposing neighbors.
		std::set<std::size_t> M;
		
		/// For black nodes, the set X, which is the set of matched neighbors.
		std::set<std::size_t> X;
		
		// Shorthands for checking whether the state is matched or running.
		bool isUR() const { return match == SIZE_MAX && running; }
		bool isMR() const { return match != SIZE_MAX && running; }
		bool isUS() const { return match == SIZE_MAX && !running; }
		bool isMS() const { return match != SIZE_MAX && !running; }
	};
	
	typedef bool Input;
	
	enum class Message { proposal, matched, accept, nothing };
	
	State init(std::size_t deg, bool black) const {
		State ret;
		
		ret.black = black;
		ret.deg = deg;
		ret.round = 0;
		ret.running = true;
		ret.match = SIZE_MAX;
		for(std::size_t i = 0; i < deg; ++i) ret.X.insert(i);
		
		return ret;
	}
	
	void send(const State& state, std::vector<Message>& msgs) const {
		std::size_t k = state.round / 2;
		
		msgs.resize(state.deg, Message::nothing);
		if(state.round % 2 == 0) {
			if(!state.black) {
				if(state.isUR() && k < state.deg) {
					msgs[k] = Message::proposal;
				}
				if(state.isMR()) {
					for(std::size_t i = 0; i < state.deg; ++i) {
						msgs[i] = Message::matched;
					}
				}
			}
		} else {
			if(state.black) {
				if(state.isUR() && !state.M.empty()) {
					msgs[*state.M.begin()] = Message::accept;
				}
			}
		}
	}
	
	State receive(State state, const std::vector<Message>& msgs) const {
		std::size_t k = state.round / 2;
		
		if(state.round % 2 == 0) {
			if(state.black) {
				if(state.isUR()) {
					for(std::size_t i = 0; i < state.deg; ++i) {
						if(msgs[i] == Message::matched) {
							state.X.erase(i);
						}
						if(msgs[i] == Message::proposal) {
							state.M.insert(i);
						}
					}
				}
			} else {
				if(state.isUR() && k >= state.deg) {
					state.running = false;
				} else if(state.isMR()) {
					state.running = false;
				}
			}
		} else {
			if(state.black) {
				if(state.isUR() && !state.M.empty()) {
					state.match = *state.M.begin();
					state.running = false;
				} else if(state.isUR() && state.X.empty()) {
					state.running = false;
				}
			} else {
				for(std::size_t i = 0; i < state.deg; ++i) {
					if(msgs[i] == Message::accept) {
						state.match = i;
						break;
					}
				}
			}
		}
		
		++state.round;
		
		return state;
	}
	
	bool stopped(const State& state) const {
		return !state.running;
	}
};

}

