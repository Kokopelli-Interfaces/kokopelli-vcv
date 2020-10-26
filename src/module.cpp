// adapted from bogaudio's module.cpp

#include "module.hpp"
#include "myrisa.hpp"

using namespace myrisa;

#define SKIN "skin"

void MyrisaModule::onReset() {
	_steps = _modulationSteps;
	reset();
}

void MyrisaModule::onSampleRateChange() {
	_modulationSteps = APP->engine->getSampleRate() * (2.5f / 1000.0f); // modulate every ~2.5ms regardless of sample rate.
	_steps = _modulationSteps;
	sampleRateChange();
}

json_t* MyrisaModule::dataToJson() {
	json_t* root = json_object();
	if (_skinnable && _skin != "default") {
		json_object_set_new(root, SKIN, json_string(_skin.c_str()));
	}
	return toJson(root);
}

void MyrisaModule::dataFromJson(json_t* root) {
	if (_skinnable) {
		json_t* s = json_object_get(root, SKIN);
		if (s) {
			setSkin(json_string_value(s));
		}
		else {
			setSkin(_skin);
		}
	}

	fromJson(root);
}

void MyrisaModule::process(const ProcessArgs& args) {
	if (!_initialized) {
		_initialized = true;
		onReset();
		onSampleRateChange();
	}

	processAlways(args);
	if (active()) {
		++_steps;
		if (_steps >= _modulationSteps) {
			_steps = 0;

			int channelsBefore = _channels;
			int channelsNow = std::max(1, channels());
			if (channelsBefore != channelsNow) {
				_channels = channelsNow;
				_inverseChannels = 1.0f / (float)_channels;
				channelsChanged(channelsBefore, channelsNow);
				if (channelsBefore < channelsNow) {
					while (channelsBefore < channelsNow) {
						addChannel(channelsBefore);
						++channelsBefore;
					}
				}
				else {
					while (channelsNow < channelsBefore) {
						removeChannel(channelsBefore - 1);
						--channelsBefore;
					}
				}
			}

			modulate();
			for (int i = 0; i < _channels; ++i) {
				modulateChannel(i);
			}
		}

		processAll(args);
		for (int i = 0; i < _channels; ++i) {
			processChannel(args, i);
		}
		postProcess(args);
	}
	postProcessAlways(args);
}

void MyrisaModule::setSkin(std::string skin) {
	if (skin == "default" || Skins::skins().validKey(skin)) {
		_skin = skin;
		for (auto scl : _skinChangeListeners) {
			scl->skinChanged(skin);
		}
	}
}

void MyrisaModule::addSkinChangeListener(SkinChangeListener* listener) {
	_skinChangeListeners.push_back(listener);
}


MyrisaModuleWidget::MyrisaModuleWidget() {
	Skins::skins().registerDefaultSkinChangeListener(this);
}

MyrisaModuleWidget::~MyrisaModuleWidget() {
	Skins::skins().deregisterDefaultSkinChangeListener(this);
}

void MyrisaModuleWidget::addParam(ParamWidget* param) {
	ModuleWidget::addParam(param);
	if (module) {
		auto l = dynamic_cast<SkinChangeListener*>(param);
		if (l) {
			auto m = dynamic_cast<MyrisaModule*>(module);
			assert(m);
			m->addSkinChangeListener(l);
		}
	}
}

void MyrisaModuleWidget::addInput(PortWidget* input) {
	ModuleWidget::addInput(input);
	if (module) {
		auto l = dynamic_cast<SkinChangeListener*>(input);
		if (l) {
			auto m = dynamic_cast<MyrisaModule*>(module);
			assert(m);
			m->addSkinChangeListener(l);
		}
	}
}

void MyrisaModuleWidget::addOutput(PortWidget* output) {
	ModuleWidget::addOutput(output);
	if (module) {
		auto l = dynamic_cast<SkinChangeListener*>(output);
		if (l) {
			auto m = dynamic_cast<MyrisaModule*>(module);
			assert(m);
			m->addSkinChangeListener(l);
		}
	}
}

void MyrisaModuleWidget::appendContextMenu(Menu* menu) {
	auto m = dynamic_cast<MyrisaModule*>(module);
	assert(m);
	if (m->_skinnable) {
		Skins* skins = &Skins::skins();
		if (skins->available().size() > 0) {
			menu->addChild(new MenuLabel());
			OptionsMenuItem* s = new OptionsMenuItem("Panel");

			s->addItem(OptionMenuItem("Default", [m]() { return m->_skin == "default"; }, [m]() { m->setSkin("default"); }));
			for (auto skin : skins->available()) {
				std::string key = skin.key;
				s->addItem(OptionMenuItem(
					skin.display.c_str(),
					[m, key]() { return m->_skin == key; },
					[m, key]() { m->setSkin(key); }
				));
			}

			s->addSpacer();
			for (auto skin : skins->available()) {
				std::string key = skin.key;
				s->addItem(OptionMenuItem(
					(std::string("Default to ") + skin.display).c_str(),
					[key, skins]() { return skins->defaultKey() == key; },
					[key, skins]() { skins->setDefaultSkin(key); }
				));
			}

			OptionsMenuItem::addToMenu(s, menu);
		}
	}

	contextMenu(menu);
}

void MyrisaModuleWidget::skinChanged(const std::string& skin) {
	updatePanel();
}

void MyrisaModuleWidget::defaultSkinChanged(const std::string& skin) {
	if (module) {
		auto m = dynamic_cast<MyrisaModule*>(module);
		assert(m);
		if (m->_skin == "default") {
			m->setSkin("default");
		}
	}
	else {
		updatePanel();
	}
}

void MyrisaModuleWidget::setPanel(Vec size, std::string slug, bool skinnable) {
	_size = size;
	_slug = slug;
	_skinnable = skinnable;
	if (module && _skinnable) {
		auto m = dynamic_cast<MyrisaModule*>(module);
		assert(m);
		m->addSkinChangeListener(this);
	}
	updatePanel();
}

void MyrisaModuleWidget::updatePanel() {
	const Skins& skins = Skins::skins();
	std::string skin = skins.defaultKey();
	if (!_skinnable) {
		skin = "light";
	}
	else if (module) {
		auto m = dynamic_cast<MyrisaModule*>(module);
		assert(m);
		skin = m->_skin;
		if (skin == "default") {
			skin = skins.defaultKey();
		}
	}

	if (_loadedSkin == skin) {
		return;
	}
	_loadedSkin = skin;
	if (_panel) {
		_panel->requestDelete();
		_panel = NULL;
	}

	std::string svg = "res/" + _slug;
	if (skin != "light") {
		svg += "-";
		svg += skin;
	}
	svg += ".svg";
	_panel = new SvgPanel();
	_panel->box.size = _size;
	addChildBottom(_panel);
	_panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, svg)));
}

void MyrisaModuleWidget::createScrews() {
	int hp = box.size.x / RACK_GRID_WIDTH;
	MyrisaModule* m = NULL;
	if (_skinnable && module) {
		m = dynamic_cast<MyrisaModule*>(module);
		assert(m);
	}
	auto screw = [m, this](Vec v) {
		auto w = createWidget<Screw>(v);
		if (m) {
			m->addSkinChangeListener(w);
		}
		this->addChild(w);
	};

	if (hp > 13) {
		screw(Vec(15, 0));
		screw(Vec(box.size.x - 30, 0));
		screw(Vec(15, 365));
		screw(Vec(box.size.x - 30, 365));
	}
	else if (hp > 8) {
		screw(Vec(0, 0));
		screw(Vec(box.size.x - 15, 0));
		screw(Vec(0, 365));
		screw(Vec(box.size.x - 15, 365));
	}
	else {
		screw(Vec(0, 0));
		screw(Vec(box.size.x - 15, 365));
	}
}
