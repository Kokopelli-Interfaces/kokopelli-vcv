#include "Signal.hpp"

int Signal::channels() {
  // TODO define # channels based on Gko layers, may be recorded with more
  // or less channels also, additional behaviours with mono or stereo
  // duplication via pos & rate will increase channels
  // best to just send # of channels from Gko
  return inputs[IN_INPUT].getChannels();
}

void Signal::modulateChannel(int c) {
  Engine &e = *_engines[c];
  e.in_attenuation = params[IN_ATTENUATION_PARAM].getValue();
  if (inputs[IN_ATTENUATION_INPUT].isConnected()) {
    e.in_attenuation *= rack::clamp(inputs[IN_ATTENUATION_INPUT].getPolyVoltage(c) / 10.f, 0.f, 1.0f);
  }

  e.out_attenuation = params[OUT_ATTENUATION_PARAM].getValue();
  if (inputs[OUT_ATTENUATION_INPUT].isConnected()) {
    e.out_attenuation *= rack::clamp(inputs[OUT_ATTENUATION_INPUT].getPolyVoltage(c) / 10.f, 0.f, 1.0f);
  }
}

void Signal::processAlways(const ProcessArgs &args) {
  outputs[OUT_OUTPUT].setChannels(_channels);
  outputs[SEL_OUTPUT].setChannels(_channels);
}

void Signal::processChannel(const ProcessArgs& args, int c) {
  Engine &e = *_engines[c];

  // TODO in_attenuation knob with SEL
  float in = inputs[IN_INPUT].getPolyVoltage(c);
  in = myrisa::dsp::attenuate(in, 1.0f - e.in_attenuation, _signal_type);

  float out = in;
  if (expanderConnected()) {
    auto toGko = toExpander();
    auto fromGko = fromExpander();

    toGko->signal[c] = in;
    toGko->signal_type = _signal_type;
    toGko->channels = _channels;
    out = myrisa::dsp::sum(fromGko->signal[c], in, _signal_type);
  }

  if (outputs[OUT_OUTPUT].isConnected()) {
    out = myrisa::dsp::attenuate(out, 1.0f - e.out_attenuation, _signal_type);
    outputs[OUT_OUTPUT].setVoltage(out, c);
  }
}

void Signal::addChannel(int c) {
  _engines[c] = new Engine();
}

void Signal::removeChannel(int c) {
  delete _engines[c];
  _engines[c] = nullptr;
}

struct SignalWidget : ModuleWidget {
	SignalWidget(Signal* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Signal.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<RoganHalfPSLightPurple>(mm2px(Vec(2.687, 50.584)), module, Signal::IN_ATTENUATION_PARAM));
		addParam(createParam<RoganHalfPSRed>(mm2px(Vec(2.687, 77.502)), module, Signal::OUT_ATTENUATION_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(3.69, 17.88)), module, Signal::IN_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 61.767)), module, Signal::IN_ATTENUATION_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(3.586, 88.685)), module, Signal::OUT_ATTENUATION_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 32.077)), module, Signal::SEL_OUTPUT));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.516, 108.703)), module, Signal::OUT_OUTPUT));
	}
};

Model* modelSignal = rack::createModel<Signal, SignalWidget>("Myrisa-Signal");
