// adapted from bogaudio's module.cpp

#include "module.hpp"
#include "kokopellivcv.hpp"

using namespace kokopellivcv;

void KokopelliVcvModule::onReset() {
	_steps = _modulationSteps;
	reset();
}

void KokopelliVcvModule::onSampleRateChange() {
	_modulationSteps = APP->engine->getSampleRate() * (2.5f / 1000.f); // modulate every ~2.5ms regardless of sample rate.
	_steps = _modulationSteps;
	sampleRateChange();
}

void KokopelliVcvModule::process(const ProcessArgs& args) {
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
