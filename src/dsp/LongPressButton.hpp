#include "rack.hpp"

namespace myrisa {
namespace dsp {

// adapted from stoermelders vcvrack-packone
// https://github.com/stoermelder/vcvrack-packone/blob/v1/src/components.hpp
struct LongPressButton {
  enum Event { NO_PRESS, SHORT_PRESS, LONG_PRESS };

  Param *param;
  float pressedTime = 0.f;
  rack::dsp::BooleanTrigger trigger;

  inline Event process(float sampleTime, float longPressThreshold = 1.f) {
    Event result = NO_PRESS;
    bool pressed = param->value > 0.f;
    if (pressed && pressedTime >= 0.f) {
      pressedTime += sampleTime;
      if (pressedTime >= longPressThreshold) {
        pressedTime = -longPressThreshold;
        result = LONG_PRESS;
      }
    }

    // Check if released
    if (trigger.process(!pressed)) {
      if (pressedTime >= 0.f) {
        result = SHORT_PRESS;
      }
      pressedTime = 0.f;
    }

    return result;
  }
};

} // namespace dsp
} // namespace myrisa
