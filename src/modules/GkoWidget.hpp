#pragma once

#include "Gko.hpp"

namespace myrisa {

struct GkoValueDisplay : TextBox {
	Gko *_module;
	int _previous_displayed_value = 0;

	GkoValueDisplay(Gko *m) : TextBox() {
    _module = m;
      // font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/nunito/Nunito-Bold.ttf"));
    // font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Overpass-Regular.ttf"));
    font = APP->window->loadFont(asset::plugin(pluginInstance, "/res/fonts/Overpass-Bold.ttf"));
    font_size = 12;
    letter_spacing = 0.f;
    // backgroundColor = nvgRGB(0xee, 0xe8, 0xd5); // solarized base2
    // backgroundColor = nvgRGB(0x93, 0xa1, 0xa1); // solarized base1
    // backgroundColor = nvgRGB(0x78, 0x78, 0x78); // blendish default
    // backgroundColor = nvgRGB(0xff, 0xff, 0xff); // custom
    backgroundColor = nvgRGB(0xd7, 0xda, 0xec); // blend
    textColor = defaultTextColor;
    box.size = mm2px(Vec(6.902, 3.283));
    textOffset = Vec(box.size.x * 0.5f, 0.f);
    textAlign = NVG_ALIGN_CENTER | NVG_ALIGN_TOP;
  }

	void setDisplayValue(int v) {
		std::string s;
		if(v != _previous_displayed_value) {
			_previous_displayed_value = v;
      s = string::f("%d", v);
      setText(s);
		}
	}
};

struct BeatDisplay : GkoValueDisplay {
  using GkoValueDisplay::GkoValueDisplay;
	void step() override {
		GkoValueDisplay::step();
		if(_module) {
      int beat = _module->_engines[0]->_timeline_position.beat;
      GkoValueDisplay::setDisplayValue(beat);
		}
	}
};

struct GkoWidget : ModuleWidget {
  const int hp = 4;

  bool _use_antipop = false;

  GkoValueDisplay *current_selection;
  GkoValueDisplay *total_selections;
  GkoValueDisplay *current_section;
  GkoValueDisplay *total_sections;
  GkoValueDisplay *current_section_division;
  GkoValueDisplay *total_section_divisions;
  BeatDisplay *time;

  GkoWidget(Gko *module) {
    setModule(module);
    box.size = Vec(RACK_GRID_WIDTH * hp, RACK_GRID_HEIGHT);
    setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Gko.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParam<Rogan1HPSWhite>(mm2px(Vec(5.333, 21.157)), module, Gko::SELECT_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.774, 33.464)), module, Gko::SELECT_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.618, 33.463)), module, Gko::SELECT_FUNCTION_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(9.64, 51.330)), module, Gko::READ_TIME_FRAME_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(1.447, 65.437)), module, Gko::RECORD_MODE_PARAM));
		addParam(createParam<MediumLEDButton>(mm2px(Vec(17.849, 65.436)), module, Gko::RECORD_TIME_FRAME_PARAM));
		addParam(createParam<Rogan3PDarkRed>(mm2px(Vec(5.334, 73.118)), module, Gko::RECORD_PARAM));

		addInput(createInput<PJ301MPort>(mm2px(Vec(8.522, 37.132)), module, Gko::SCENE_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(8.384, 88.869)), module, Gko::RECORD_INPUT));
		addInput(createInput<PJ301MPort>(mm2px(Vec(1.798, 108.114)), module, Gko::PHASE_INPUT));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.306, 108.114)), module, Gko::PHASE_OUTPUT));

		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(3.083, 34.928)), module, Gko::SELECT_FUNCTION_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.239, 34.928)), module, Gko::SELECT_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.155, 52.736)), module, Gko::READ_TIME_FRAME_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.097, 62.77)), module, Gko::RECORD_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(2.912, 66.901)), module, Gko::RECORD_MODE_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(19.313, 66.901)), module, Gko::RECORD_TIME_FRAME_LIGHT));
		addChild(createLight<MediumLight<RedGreenBlueLight>>(mm2px(Vec(11.181, 110.546)), module, Gko::PHASE_LIGHT));

    auto display_size = mm2px(Vec(9.096, 4.327));

		current_selection = new GkoValueDisplay(module);
    current_selection->box.pos = mm2px(Vec(2.921, 16.235));
    current_selection->box.size = display_size;
    addChild(current_selection);

    total_selections = new GkoValueDisplay(module);
    total_selections->box.pos = mm2px(Vec(13.387, 16.236));
    total_selections->box.size = display_size;
    addChild(total_selections);

    display_size = mm2px(Vec(6.837, 4.327));

    current_section = new GkoValueDisplay(module);
    current_section->box.pos = mm2px(Vec(1.071, 48.917));
    current_section->box.size = display_size;
    addChild(current_section);

    current_section_division = new GkoValueDisplay(module);
    current_section_division->box.pos = mm2px(Vec(17.511, 48.917));
    current_section_division->box.size = display_size;
    addChild(current_section_division);

    total_sections = new GkoValueDisplay(module);
    total_sections->box.pos = mm2px(Vec(1.071, 54.022));
    total_sections->box.size = display_size;
    addChild(total_sections);

    total_section_divisions = new GkoValueDisplay(module);
    total_section_divisions->box.pos = mm2px(Vec(17.511, 54.022));
    total_section_divisions->box.size = display_size;
    addChild(total_section_divisions);

    display_size = mm2px(Vec(21.44, 4.327));

    time = new BeatDisplay(module);
    time->box.pos = mm2px(Vec(1.974, 99.568));
    time->box.size = display_size;
    time->textOffset = Vec(time->box.size.x * 0.5f, 0.f);
    addChild(time);
}

	void appendContextMenu(rack::Menu* menu) override {
		auto m = dynamic_cast<Gko*>(module);
		assert(m);

    menu->addChild(new MenuLabel());

    menu->addChild(new BoolOptionMenuItem("Antipop Filter", [m]() {
      return &m->_options.use_antipop;
    }));


    // menu->addChild(new Slider());
};

};

} // namespace myrisa
