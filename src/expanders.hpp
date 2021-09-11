// adapted from bogaudios's src/expanders.hpp

#pragma once

#include <type_traits>
#include <functional>

#include "rack.hpp"
#include "module.hpp"

using namespace rack;

namespace tribalinterfaces {

struct ExpanderMessage {
	int channels = 0;

	virtual ~ExpanderMessage() {}
};

template<class MSG, class BASE>
struct ExpandableModule : BASE {
	std::function<bool(Model*)> _expanderModel;
	MSG _messages[2] {};
	bool _wasConnected = false;

	ExpandableModule() {
		static_assert(std::is_base_of<ExpanderMessage, MSG>::value, "type parameter MSG must derive from ExpanderMessage");
		static_assert(std::is_base_of<TribalInterfacesModule, BASE>::value, "type parameter BASE must derive from TribalInterfacesModule");

		TribalInterfacesModule::rightExpander.producerMessage = &_messages[0];
		TribalInterfacesModule::rightExpander.consumerMessage = &_messages[1];
	}

	void setExpanderModelPredicate(std::function<bool(Model*)> p) {
		_expanderModel = p;
	}

	bool expanderConnected() {
		bool connected = TribalInterfacesModule::rightExpander.module && _expanderModel && _expanderModel(TribalInterfacesModule::rightExpander.module->model);
		if (!connected && _wasConnected) {
			_messages[1] = _messages[0] = MSG();
		}
		return _wasConnected = connected;
	}

	inline MSG* toExpander() {
		return (MSG*)TribalInterfacesModule::rightExpander.module->leftExpander.producerMessage;
	}

	inline MSG* fromExpander() {
		return (MSG*)TribalInterfacesModule::rightExpander.consumerMessage;
	}

	void process(const TribalInterfacesModule::ProcessArgs& args) override {
		BASE::process(args);
		if (TribalInterfacesModule::rightExpander.module) {
			TribalInterfacesModule::rightExpander.module->leftExpander.messageFlipRequested = true;
		}
	}
};

// An expander must be to the right of the expanded module to work.
template<class MSG, class BASE>
struct ExpanderModule : BASE {
	std::function<bool(Model*)>  _baseModel;
	MSG _messages[2] {};
	bool _wasConnected = false;

	ExpanderModule() {
		static_assert(std::is_base_of<ExpanderMessage, MSG>::value, "type parameter MSG must derive from ExpanderMessage");
		static_assert(std::is_base_of<TribalInterfacesModule, BASE>::value, "type parameter BASE must derive from TribalInterfacesModule");

		TribalInterfacesModule::leftExpander.producerMessage = &_messages[0];
		TribalInterfacesModule::leftExpander.consumerMessage = &_messages[1];
	}

	void setBaseModelPredicate(std::function<bool(Model*)> p) {
		_baseModel = p;
	}

	bool baseConnected() {
		bool connected = TribalInterfacesModule::leftExpander.module && _baseModel && _baseModel(TribalInterfacesModule::leftExpander.module->model);
		if (!connected && _wasConnected) {
			_messages[1] = _messages[0] = MSG();
		}
		return _wasConnected = connected;
	}

	inline MSG* fromBase() {
		return (MSG*)TribalInterfacesModule::leftExpander.consumerMessage;
	}

	inline MSG* toBase() {
		return (MSG*)TribalInterfacesModule::leftExpander.module->rightExpander.producerMessage;
	}

  // TODO instead, define based off frame
	// int channels() override final {
	// 	if (baseConnected()) {
	// 		return fromBase()->channels;
	// 	}
	// 	return 1;
	// }

	void process(const TribalInterfacesModule::ProcessArgs& args) override {
		BASE::process(args);
		if (TribalInterfacesModule::leftExpander.module) {
			TribalInterfacesModule::leftExpander.module->rightExpander.messageFlipRequested = true;
		}
	}
};
} // namespace tribalinterfaces
