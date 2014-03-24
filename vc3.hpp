#pragma once

#include "bmm.hpp"

namespace dda_simulator {

/// Port numbered VC3 machine that finds a 3-approximation of a minimum
/// vertex cover of a simple graph. The algorithm ignores its input.
/// The algorithm is described in
///   Jukka Suomela: A Course on Deterministic Distributed Algorithms
///   http://users.ics.aalto.fi/suomela/dda/
/// The output vertex cover may be read using the isInVertexCover methods
/// of the states.
struct VC3Machine {
	struct State {
		/// State of the first part of the vertex.
		BMMMachine::State state1;
		
		/// State of the second part of the vertex.
		BMMMachine::State state2;
		
		bool isInVertexCover() const {
			return state1.match != SIZE_MAX || state2.match != SIZE_MAX;
		}
	};
	
	// The input is ignored.
	typedef bool Input;
	
	typedef std::pair<BMMMachine::Message, BMMMachine::Message> Message;
	
	State init(std::size_t deg, bool input) const {
		State ret;
		
		ret.state1 = BMMMachine().init(deg, false);
		ret.state2 = BMMMachine().init(deg, true);
		
		return ret;
	}
	
	void send(const State& state, std::vector<Message>& msgs) const {
		std::vector<BMMMachine::Message> msgs1;
		std::vector<BMMMachine::Message> msgs2;
		
		BMMMachine().send(state.state1, msgs2);
		BMMMachine().send(state.state2, msgs1);
		
		for(std::size_t i = 0; i < msgs1.size(); ++i) {
			msgs.emplace_back(msgs1[i], msgs2[i]);
		}
	}
	
	State receive(State state, const std::vector<Message>& msgs) const {
		std::vector<BMMMachine::Message> msgs1;
		std::vector<BMMMachine::Message> msgs2;
		
		for(const Message& msg : msgs) {
			msgs1.push_back(msg.first);
			msgs2.push_back(msg.second);
		}
		
		state.state1 = BMMMachine().receive(state.state1, msgs1);
		state.state2 = BMMMachine().receive(state.state2, msgs2);
		
		return state;
	}
	
	bool stopped(const State& state) const {
		return !state.state1.running && !state.state2.running;
	}
};

}

