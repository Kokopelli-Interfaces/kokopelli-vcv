#pragma once

// #include <boost/circular_buffer.hpp>
#include <rack.hpp>
#include <cmath>

namespace myrisa {
namespace dsp {

struct AntipopFilter {

  const float click_threshold =  0.1f;

  // boost::circular_buffer<float> lookback(50);

  int num_samples_faded = 0;
  int num_samples_to_fade = 0;
  float sample_before_discontinuity = 0.f;
  float last_sample = 0.f;

  inline float process(float sample) {
    float out = sample;

    if (num_samples_to_fade == 0 && click_threshold < std::fabs(sample - last_sample)) {
      sample_before_discontinuity = last_sample;
      float discontinuity_distance = std::fabs(sample - last_sample);
      num_samples_to_fade = (int) (discontinuity_distance * 100);
      printf("triggered! disc: %f, fade %d samples \n", discontinuity_distance, num_samples_to_fade);
    }

    last_sample = sample;

    if (num_samples_faded < num_samples_to_fade) {
      float fade = (float)num_samples_faded / (float)num_samples_to_fade;
      out = rack::crossfade(sample_before_discontinuity, sample, fade);
      num_samples_faded++;

      if (num_samples_faded == num_samples_to_fade) {
        num_samples_faded = 0;
        num_samples_to_fade = 0;
        printf("anti pop recovered");
      }
    }

    return out;
  }
};

} // namespace dsp
} // namespace myrisa
