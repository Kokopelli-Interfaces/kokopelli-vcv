#pragma once

namespace myrisa {
namespace util {

inline double rescale(double x2, double x1, double y2, double y1) {
  return (x2 - x1) / (y2 - y1);
}

} // namespace util
} // namespace myrisa
