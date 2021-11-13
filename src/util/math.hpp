#pragma once

#include <vector>

namespace kokopellivcv {
namespace util {

inline int findNumberAfterFirstThatFitsIntoSecond(int n1, int n2) {
  assert(n1 < n2);

  while (n2 % n1 != 0) {
    n1++;
    if (n1 == n2) {
      return n1;
    }
  }

  return n1;
}

} // namespace util
} // namespace kokopellivcv
