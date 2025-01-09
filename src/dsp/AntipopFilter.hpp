// adapted from Lomas Modules src/dsp/Antipop

#pragma once

namespace kokopellivcv {
namespace dsp {

struct AntipopFilter {

  float _alpha = 0.00001f;
  float _filter;

  inline void trigger() {
    _alpha = 0.f;
  }

  inline float process(float in, bool recover) {
    if (_alpha >= 1.0f) {
        _filter = in;
        return in;
    }

    if (recover) {
      _alpha += 1.f / 30.f; // recovers in ~30 samples
    }

    _filter += _alpha * (in - _filter);

    return _filter;
  }
};

} // namespace dsp
} // namespace kokopellivcv
