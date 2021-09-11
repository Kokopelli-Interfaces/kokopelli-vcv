#pragma once

#include "dsp/Signal.hpp"
#include "Signal.hpp"
#include "menu.hpp"

using namespace tribalinterfaces::dsp;

namespace tribalinterfaces {

struct SignalWidget : ModuleWidget {
	SignalWidget(Signal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Signal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSLightBrown>(mm2px(Vec(2.687, 50.584)), module, Signal::IN_ATTENUATION_PARAM));
		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(2.687, 77.502)), module, Signal::OUT_ATTENUATION_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.69, 17.88)), module, Signal::IN_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 61.767)), module, Signal::IN_ATTENUATION_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 88.685)), module, Signal::OUT_ATTENUATION_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 32.077)), module, Signal::SEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 108.703)), module, Signal::OUT_OUTPUT));
	}

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Signal*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

		OptionsMenuItem* signal_type_menu = new OptionsMenuItem("Signal Type");

    signal_type_menu->addItem(OptionMenuItem("Audio", [m]() { return m->_signal_type == SignalType::AUDIO; }, [m]() { m->_signal_type = SignalType::AUDIO; }));
		signal_type_menu->addItem(OptionMenuItem("Parameter", [m]() { return m->_signal_type == SignalType::PARAM; }, [m]() { m->_signal_type = SignalType::PARAM; }));
		signal_type_menu->addItem(OptionMenuItem("Gate", [m]() { return m->_signal_type == SignalType::GATE; }, [m]() { m->_signal_type = SignalType::GATE; }));
		signal_type_menu->addItem(OptionMenuItem("Control Voltage", [m]() { return m->_signal_type == SignalType::CV; }, [m]() { m->_signal_type = SignalType::CV; }));
		signal_type_menu->addItem(OptionMenuItem("Pitch", [m]() { return m->_signal_type == SignalType::VOCT; }, [m]() { m->_signal_type = SignalType::VOCT; }));
		signal_type_menu->addItem(OptionMenuItem("Velocity", [m]() { return m->_signal_type == SignalType::VEL; }, [m]() { m->_signal_type = SignalType::VEL; }));

		OptionsMenuItem::addToMenu(signal_type_menu, menu);
  }
};


} // namespace tribalinterfaces
