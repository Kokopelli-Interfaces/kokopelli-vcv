#pragma once

namespace kokopelli {
namespace dsp {

const float saturation_limit = 12.0f;

// Zavalishin 2018, "The Art of VA Filter Design", http://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_2.0.0a.pdf
inline float saturation(float x) {
	const float y1 = 0.98765f; // (2*x - 1)/x**2 where x is 0.9.
	const float offset = 0.075f / saturation_limit; // magic.
	float x1 = (x + 1.0f) * 0.5f;
	return saturation_limit * (offset + x1 - sqrtf(x1 * x1 - y1 * x) * (1.0f / y1));
}

inline float saturate(float sample) {
	float x = sample * (1.0f / saturation_limit);
	if (sample < 0.0f) {
		return -saturation(-x);
	}
	return saturation(x);
}

} // namespace dsp
} // namespace kokopelli
