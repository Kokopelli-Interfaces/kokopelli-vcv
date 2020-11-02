#include "Signal.hpp"

float getMixOut(float mix, float in, float frame_out, float frame_sel_out) {
  return frame_out;
}

void Signal::processAlways(const ProcessArgs &args) {
  // float *mix = inputs[MIX_INPUT].getVoltages();

  SignalExpanderMessage *toFrame = &_dummyExpanderMessage;
  SignalExpanderMessage *fromFrame = &_dummyExpanderMessage;
  if (expanderConnected()) {
    toFrame = toExpander();
    fromFrame = fromExpander();
  }

  int n = inputs[IN_INPUT].getChannels();
  for (int c = 0; c < n; ++c) {
    toFrame->signal[c] = inputs[IN_INPUT].getPolyVoltage(c);

    if (outputs[OUT_OUTPUT].isConnected()) {
      float frame_out = fromFrame->signal[c];

      float mix = clamp(params[MIX_PARAM].getValue(), 0.0f, 1.0f);
      float in = inputs[IN_INPUT].getPolyVoltage(c);
      float mix_out = getMixOut(mix, in, frame_out, 0.0f);

      float vca = clamp(params[VCA_PARAM].getValue(), 0.0f, 1.0f);
      outputs[OUT_OUTPUT].setVoltage(mix_out * vca, c);
    }
  }
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
