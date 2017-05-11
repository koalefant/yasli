/**
 *  yasli - Serialization Library.
 *  Copyright (C) 2007-2017 Evgeny Andreeshchev <eugene.andreeshchev@gmail.com>
 *                          Alexander Kotliar <alexander.kotliar@gmail.com>
 * 
 *  This code is distributed under the MIT License:
 *                          https://www.opensource.org/licenses/MIT
 */
#pragma once

#include <vector>
#include <memory> // std::unique_ptr
#include <utility> // std::forward

struct SignalScope;

namespace signal_details {

struct ConnectionBase {
	virtual ~ConnectionBase();
	SignalScope* scope { nullptr };

	ConnectionBase() = default;
	ConnectionBase(SignalScope* scope);
};

template<class ...Args>
struct Connection : ConnectionBase {
	virtual void call(Args... args) = 0;

	Connection(SignalScope* scope) : ConnectionBase(scope) {}
};

template<class Functor, class ...Args>
struct ConnectionFunctor : Connection<Args...> {
	Functor functor;

	ConnectionFunctor(SignalScope* scope, Functor&& functor)
	: Connection<Args...>(scope)
	, functor(std::forward<Functor>(functor))
	{}

	void call(Args... args) override { functor(args...); }
};

template<class Object, class ...Args>
struct ConnectionMemberFunction : Connection<Args...> {
	typedef void (Object::*Member)(Args...);
	Object* object { nullptr };
	Member member { nullptr };

	ConnectionMemberFunction(SignalScope* scope, Object* object, Member member)
	: Connection<Args...>(scope)
	, object(object)
	, member(member)
	{}

	void call(Args... args) override { (object->*member)(std::forward<Args>(args)...); }
};

struct SignalBase {
	std::vector<std::unique_ptr<ConnectionBase>> connections;
	~SignalBase() { disconnect_all(); }

	void disconnect(SignalScope* scope);
	void disconnect_all();
};

}

// will disconnect signals on destruction
struct SignalScope {
	~SignalScope() { disconnect_all(); }

	SignalScope() = default;
	SignalScope(const SignalScope& rhs) = delete;
	SignalScope& operator=(SignalScope& rhs) = delete;

	void disconnect_all();

	std::vector<signal_details::SignalBase*> connected_signals;
private:
	void disconnect_self(signal_details::SignalBase* signal_base);
	friend struct signal_details::SignalBase;
};

template<class ...Args>
struct Signal : signal_details::SignalBase {
#ifndef emit // in case we are using Qt defines
	void emit(Args... args) { operator()(std::forward<Args>(args)...); }
#endif

	Signal() = default;
	Signal(const Signal&) = delete;
	Signal& operator=(const Signal& rhs) = delete;

	void operator()(Args... args) { 
		int num_connections = connections.size();
		for (int i = 0; i < num_connections; ++i) {
			auto* connection = static_cast<signal_details::Connection<Args...>*>(connections[i].get());
			connection->call(std::forward<Args>(args)...);
		}
	}

	template<class ScopeDerived>
	void connect(ScopeDerived* object, void (ScopeDerived::*member)(Args...)) {
		static_assert(std::is_base_of<SignalScope, ScopeDerived>::value,
					  "Connecting to instance that is not derived from SignalScope, consider inheriting SignalScope or specifying scope explicitly with 3 argument overload.");
		SignalScope* scope = static_cast<ScopeDerived*>(object);
		connections.emplace_back(std::unique_ptr<signal_details::ConnectionBase>{
			new signal_details::ConnectionMemberFunction<ScopeDerived, Args...>(scope, object, member)
		});
		scope->connected_signals.push_back(this);
	}

	template<class Object>
	void connect(SignalScope* scope, Object* object, void (Object::*member)(Args...)) {
		connections.emplace_back(std::unique_ptr<signal_details::ConnectionBase>{
			new signal_details::ConnectionMemberFunction<Object, Args...>(scope, object, member)
		});
		scope->connected_signals.push_back(this);
	}

	template<class Functor>
	void connect(SignalScope* scope, Functor&& functor) {
		connections.emplace_back(std::unique_ptr<signal_details::ConnectionBase>{ 
			new signal_details::ConnectionFunctor<Functor, Args...>(scope, std::forward<Functor>(functor))
		});
		scope->connected_signals.push_back(this);
	}
};
