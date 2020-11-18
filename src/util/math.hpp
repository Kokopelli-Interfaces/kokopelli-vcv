#pragma once

namespace myrisa {
namespace util {

inline float rescale(float x2, float x1, float y2, float y1) {
  return (x2 - x1) / (y2 - y1);
}

} // namespace util
} // namespace myrisa
