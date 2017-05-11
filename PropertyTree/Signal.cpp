/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2017 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          https://www.opensource.org/licenses/MIT
 */
#include "Signal.h"

#include <algorithm> // std::remove

void SignalScope::disconnect_all() {
	int num_signals = connected_signals.size();
	for (int i = 0; i < num_signals; ++i) {
		connected_signals[i]->disconnect(this);
	}
}

void SignalScope::disconnect_self(signal_details::SignalBase* signal_base) {
	connected_signals.erase(std::remove(connected_signals.begin(),
										connected_signals.end(), signal_base),
							connected_signals.end());
} 

namespace signal_details {

ConnectionBase::~ConnectionBase() {
}

ConnectionBase::ConnectionBase(SignalScope* scope)
: scope(scope) {
}


void SignalBase::disconnect(SignalScope* scope) {
	int num_connections = connections.size();
	for (int i = 0; i < num_connections; ++i) {
		if (connections[i]->scope == scope) {
			scope->disconnect_self(this);
			connections[i]->scope = nullptr;
			connections[i].reset();
		}	
	}
	connections.erase(std::remove(connections.begin(),
								  connections.end(), std::unique_ptr<ConnectionBase>()),
					  connections.end());
} 

void SignalBase::disconnect_all() {
	int num_connections = connections.size();
	for (int i = 0; i < num_connections; ++i) {
		SignalScope* scope = connections[i]->scope;
		if (scope == nullptr)
			continue;
		auto& s = scope->connected_signals;
		s.erase(std::remove(s.begin(), s.end(), this), s.end());
		connections[i].reset();
	}
	connections.clear();
} 

}
