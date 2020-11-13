#pragma once

#include <math.h>
#include <vector>

#include "assert.hpp"
#include "dsp/Antipop.hpp"
#include "dsp/Interpolation.hpp"

using namespace std;

namespace myrisa {

// a buffer that can be read and written to via a phase float in [0.0f, 1.0f]
struct PhaseBuffer {
private:
  vector<float> buffer;
  AntipopFilter antipop_filter;
public:
  void setSize(int size) {
    buffer.resize(size);
  }

  int getSize() {
    return buffer.size();
  }

  void append(float sample) {
    buffer.push_back(sample);
  }

  void replace(double phase, float sample) {
    ASSERT(0, <, buffer.size());
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

    int length = buffer.size();
    float position = length * phase;
    int i1 = floor(position) == length ? floor(position) : length - 1;
    int i2 = ceil(position) == length ? ceil(position) : 0;
    float w = position - i1;

    // TODO different more sophisticated ways to write?
    buffer[i1] += sample * (1 - w);
    buffer[i2] += sample * (w);
  }

  float read(double phase, Interpolations interpolation_mode) {
    ASSERT(0.0f, <=, phase);
    ASSERT(phase, <=, 1.0f);

    int size = getSize();
    if (size == 0) {
      return 0.0f;
    }

    double buffer_position = size * phase;

    switch (interpolation_mode) {
    case NONE:
      return buffer[floor(buffer_position)];
      break;
    case LINEAR:
      return interpolateLinearD(buffer.data(), buffer_position);
      break;
    case HERMITE:
      return InterpolateHermite(buffer.data(), buffer_position, size);
      break;
    case BSPLINE:
      return interpolateBSpline(buffer.data(), buffer_position);
      break;
    default:
      return buffer[floor(buffer_position)];
    }
  }
};

} // namepsace myrisa
