#include "plugin.hpp"

template <class TChoice>
struct Grid16MidiWidget : MidiWidget {
	LedDisplaySeparator* hSeparators[4];
	LedDisplaySeparator* vSeparators[4];
	TChoice* choices[4][4];

	template <class TModule>
	void setModule(TModule* module) {
		Vec pos = channelChoice->box.getBottomLeft();
		// Add vSeparators
		for (int x = 1; x < 4; x++) {
			vSeparators[x] = createWidget<LedDisplaySeparator>(pos);
			vSeparators[x]->box.pos.x = box.size.x / 4 * x;
			addChild(vSeparators[x]);
		}
		// Add hSeparators and choice widgets
		for (int y = 0; y < 4; y++) {
			hSeparators[y] = createWidget<LedDisplaySeparator>(pos);
			hSeparators[y]->box.size.x = box.size.x;
			addChild(hSeparators[y]);
			for (int x = 0; x < 4; x++) {
				choices[x][y] = new TChoice;
				choices[x][y]->box.pos = pos;
				choices[x][y]->setId(4 * y + x);
				choices[x][y]->box.size.x = box.size.x / 4;
				choices[x][y]->box.pos.x = box.size.x / 4 * x;
				choices[x][y]->setModule(module);
				addChild(choices[x][y]);
			}
			pos = choices[0][y]->box.getBottomLeft();
		}
		for (int x = 1; x < 4; x++) {
			vSeparators[x]->box.size.y = pos.y - vSeparators[x]->box.pos.y;
		}
	}
};

template <class TModule>
struct CcChoice : LedDisplayChoice {
	TModule* module;
	int id;
	int focusCc;

	CcChoice() {
		box.size.y = mm2px(6.666);
		textOffset.y -= 4;
	}

	void setModule(TModule* module) {
		this->module = module;
	}

	void setId(int id) {
		this->id = id;
	}

	void step() override {
		int cc;
		if (!module) {
			cc = id;
		}
		else if (module->learningId == id) {
			cc = focusCc;
			color.a = 0.5;
		}
		else {
			cc = module->learnedCcs[id];
			color.a = 1.0;

			// Cancel focus if no longer learning
			if (APP->event->getSelectedWidget() == this)
				APP->event->setSelected(NULL);
		}

		// Set text
		if (cc < 0)
			text = "--";
		else
			text = string::f("%d", cc);
	}

	void onSelect(const event::Select& e) override {
		if (!module)
			return;
		module->learningId = id;
		focusCc = -1;
		e.consume(this);
	}

	void onDeselect(const event::Deselect& e) override {
		if (!module)
			return;
		if (module->learningId == id) {
			if (0 <= focusCc && focusCc < 128) {
				module->learnedCcs[id] = focusCc;
			}
			module->learningId = -1;
		}
	}

	void onSelectText(const event::SelectText& e) override {
		int c = e.codepoint;
		if ('0' <= c && c <= '9') {
			if (focusCc < 0)
				focusCc = 0;
			focusCc = focusCc * 10 + (c - '0');
		}
		if (focusCc >= 128)
			focusCc = -1;
		e.consume(this);
	}

	void onSelectKey(const event::SelectKey& e) override {
		if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && e.action == GLFW_PRESS && (e.mods & RACK_MOD_MASK) == 0) {
			event::Deselect eDeselect;
			onDeselect(eDeselect);
			APP->event->selectedWidget = NULL;
			e.consume(this);
		}
	}
};


struct MIDI_CC : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CC_OUTPUT, 16),
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	midi::InputQueue midiInput;
	int8_t values[128][16];
	int learningId;
	int learnedCcs[16];
	dsp::ExponentialFilter valueFilters[16][16];

	MIDI_CC() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		for (int i = 0; i < 16; i++) {
      for (int c = 0; c < 16; c++) {
        valueFilters[i][c].setTau(1 / 30.f);
      }
		}
		onReset();
	}

	void onReset() override {
		for (int i = 0; i < 128; i++) {
      for (int c = 0; c < 16; c++) {
        values[i][c] = 0;
      }
		}
		for (int i = 0; i < 16; i++) {
			learnedCcs[i] = i;
		}
		learningId = -1;
		midiInput.reset();
	}

	void process(const ProcessArgs& args) override {
		midi::Message msg;
		while (midiInput.shift(&msg)) {
			processMessage(msg);
		}

		for (int i = 0; i < 16; i++) {
			if (!outputs[CC_OUTPUT + i].isConnected())
				continue;

      outputs[CC_OUTPUT + i].setChannels(16);
      int cc = learnedCcs[i];
      for (int c = 0; c < 16; c++) {
        float value = values[cc][c] / 127.f;

        // u are a bad buuuooy
        if (value - .5 > 0 && value - .5 < .01) {
          value = .5;
        }

        // Detect behavior from MIDI buttons.
        if (std::fabs(valueFilters[i][c].out - value) >= 1.f) {
          // Jump value
          valueFilters[i][c].out = value;
        }
        else {
          // Smooth value with filter
          valueFilters[i][c].process(args.sampleTime, value);
        }
        outputs[CC_OUTPUT + i].setVoltage(valueFilters[i][c].out * 10.f, c);
      }
		}

	}

	void processMessage(midi::Message msg) {
		switch (msg.getStatus()) {
			// cc
			case 0xb: {
				processCC(msg);
			} break;
			default: break;
		}
	}

	void processCC(midi::Message msg) {
		uint8_t cc = msg.getNote();
		// Allow CC to be negative if the 8th bit is set.
		// The gamepad driver abuses this, for example.
		// Cast uint8_t to int8_t
		int8_t value = msg.bytes[2];
    int8_t channel = msg.bytes[0] & 0x0f;
		value = clamp(value, -127, 127);
		// Learn
		if (learningId >= 0 && values[cc][0] != value) {
			learnedCcs[learningId] = cc;
			learningId = -1;
		}
		values[cc][channel] = value;
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* ccsJ = json_array();
		for (int i = 0; i < 16; i++) {
			json_array_append_new(ccsJ, json_integer(learnedCcs[i]));
		}
		json_object_set_new(rootJ, "ccs", ccsJ);

		// Remember values so users don't have to touch MIDI controller knobs when restarting Rack
		json_t* valuesJ = json_array();
		for (int i = 0; i < 128; i++) {
      for (int c = 0; c < 16; c++) {
        json_array_append_new(valuesJ, json_integer(values[i][c]));
      }
		}
		json_object_set_new(rootJ, "values", valuesJ);

		json_object_set_new(rootJ, "midi", midiInput.toJson());
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		json_t* ccsJ = json_object_get(rootJ, "ccs");
		if (ccsJ) {
			for (int i = 0; i < 16; i++) {
				json_t* ccJ = json_array_get(ccsJ, i);
				if (ccJ)
					learnedCcs[i] = json_integer_value(ccJ);
			}
		}

		json_t* valuesJ = json_object_get(rootJ, "values");
		if (valuesJ) {
			for (int i = 0; i < 128; i++) {
        for (int c = 0; c < 16; c++) {
          json_t* valueJ = json_array_get(valuesJ, i*16 + c);
          if (valueJ) {
            values[i][c] = json_integer_value(valueJ);
          }
        }
			}
		}

		json_t* midiJ = json_object_get(rootJ, "midi");
		if (midiJ)
			midiInput.fromJson(midiJ);
	}
};


struct MIDI_CCWidget : ModuleWidget {
	MIDI_CCWidget(MIDI_CC* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::system("res/Core/MIDI-CC.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 73.344704)), module, MIDI_CC::CC_OUTPUT + 0));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 73.344704)), module, MIDI_CC::CC_OUTPUT + 1));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 73.344704)), module, MIDI_CC::CC_OUTPUT + 2));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 73.344704)), module, MIDI_CC::CC_OUTPUT + 3));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943355, 84.945023)), module, MIDI_CC::CC_OUTPUT + 4));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 84.945023)), module, MIDI_CC::CC_OUTPUT + 5));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.094982, 84.945023)), module, MIDI_CC::CC_OUTPUT + 6));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 84.945023)), module, MIDI_CC::CC_OUTPUT + 7));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.8943343, 96.543976)), module, MIDI_CC::CC_OUTPUT + 8));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.494659, 96.543976)), module, MIDI_CC::CC_OUTPUT + 9));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 96.543976)), module, MIDI_CC::CC_OUTPUT + 10));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 96.543976)), module, MIDI_CC::CC_OUTPUT + 11));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(3.894335, 108.14429)), module, MIDI_CC::CC_OUTPUT + 12));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(15.49466, 108.14429)), module, MIDI_CC::CC_OUTPUT + 13));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(27.09498, 108.14429)), module, MIDI_CC::CC_OUTPUT + 14));
		addOutput(createOutput<PJ301MPort>(mm2px(Vec(38.693932, 108.14429)), module, MIDI_CC::CC_OUTPUT + 15));

		typedef Grid16MidiWidget<CcChoice<MIDI_CC>> TMidiWidget;
		TMidiWidget* midiWidget = createWidget<TMidiWidget>(mm2px(Vec(3.399621, 14.837339)));
		midiWidget->box.size = mm2px(Vec(44, 54.667));
		midiWidget->setMidiPort(module ? &module->midiInput : NULL);
		midiWidget->setModule(module);
		addChild(midiWidget);
	}
};

Model *modelMyrisaMidiCC = createModel<MIDI_CC, MIDI_CCWidget>("MyrisaMidiCC");
