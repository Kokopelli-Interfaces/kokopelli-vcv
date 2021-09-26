// adapted from bogaudios's src/expanders.hpp

#pragma once

#include <type_traits>
#include <functional>

#include "rack.hpp"
#include "module.hpp"

using namespace rack;

namespace kokpelliinterfaces {

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
		static_assert(std::is_base_of<KokpelliInterfacesModule, BASE>::value, "type parameter BASE must derive from KokpelliInterfacesModule");

		KokpelliInterfacesModule::rightExpander.producerMessage = &_messages[0];
		KokpelliInterfacesModule::rightExpander.consumerMessage = &_messages[1];
	}

	void setExpanderModelPredicate(std::function<bool(Model*)> p) {
		_expanderModel = p;
	}

	bool expanderConnected() {
		bool connected = KokpelliInterfacesModule::rightExpander.module && _expanderModel && _expanderModel(KokpelliInterfacesModule::rightExpander.module->model);
		if (!connected && _wasConnected) {
			_messages[1] = _messages[0] = MSG();
		}
		return _wasConnected = connected;
	}

	inline MSG* toExpander() {
		return (MSG*)KokpelliInterfacesModule::rightExpander.module->leftExpander.producerMessage;
	}

	inline MSG* fromExpander() {
		return (MSG*)KokpelliInterfacesModule::rightExpander.consumerMessage;
	}

	void process(const KokpelliInterfacesModule::ProcessArgs& args) override {
		BASE::process(args);
		if (KokpelliInterfacesModule::rightExpander.module) {
			KokpelliInterfacesModule::rightExpander.module->leftExpander.messageFlipRequested = true;
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
		static_assert(std::is_base_of<KokpelliInterfacesModule, BASE>::value, "type parameter BASE must derive from KokpelliInterfacesModule");

		KokpelliInterfacesModule::leftExpander.producerMessage = &_messages[0];
		KokpelliInterfacesModule::leftExpander.consumerMessage = &_messages[1];
	}

	void setBaseModelPredicate(std::function<bool(Model*)> p) {
		_baseModel = p;
	}

	bool baseConnected() {
		bool connected = KokpelliInterfacesModule::leftExpander.module && _baseModel && _baseModel(KokpelliInterfacesModule::leftExpander.module->model);
		if (!connected && _wasConnected) {
			_messages[1] = _messages[0] = MSG();
		}
		return _wasConnected = connected;
	}

	inline MSG* fromBase() {
		return (MSG*)KokpelliInterfacesModule::leftExpander.consumerMessage;
	}

	inline MSG* toBase() {
		return (MSG*)KokpelliInterfacesModule::leftExpander.module->rightExpander.producerMessage;
	}

  // TODO instead, define based off frame
	// int channels() override final {
	// 	if (baseConnected()) {
	// 		return fromBase()->channels;
	// 	}
	// 	return 1;
	// }

	void process(const KokpelliInterfacesModule::ProcessArgs& args) override {
		BASE::process(args);
		if (KokpelliInterfacesModule::leftExpander.module) {
			KokpelliInterfacesModule::leftExpander.module->rightExpander.messageFlipRequested = true;
		}
	}
};
} // namespace kokpelliinterfaces
