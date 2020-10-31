#include "Signal.hpp"

void Signal::processAlways(const ProcessArgs &args) {
  float *input = inputs[IN_INPUT].getVoltages();
  float *mix = inputs[MIX_INPUT].getVoltages();
  float *vca = inputs[VCA_INPUT].getVoltages();
}

struct SignalWidget : ModuleWidget {
	SignalWidget(Signal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Signal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSLightPurple>(mm2px(Vec(2.687, 50.584)), module, Signal::MIX_PARAM));
		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(2.687, 77.502)), module, Signal::VCA_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.69, 17.88)), module, Signal::IN_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 61.767)), module, Signal::MIX_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 88.685)), module, Signal::VCA_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 32.077)), module, Signal::SEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 108.703)), module, Signal::OUT_OUTPUT));
	}
};

Model* modelSignal = rack::createModel<Signal, SignalWidget>("Myrisa-Signal");
