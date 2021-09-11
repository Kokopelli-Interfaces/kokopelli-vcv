#pragma once

#include <vector>

namespace tribalinterfaces {
namespace util {

inline double rescale(double x2, double x1, double y2, double y1) {
  return (x2 - x1) / (y2 - y1);
}

inline unsigned int gcd(unsigned int a,unsigned int b) {
    int temp;
    while(b > 0) {
        temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

inline unsigned int gcd(std::vector<unsigned int> &numbers) {
  if (numbers.size() == 0) {
    return 0;
  }

  int result = numbers[0];
  for(unsigned int i=1; i<numbers.size(); i++) {
    result = gcd(result, numbers[i]);
  }
  return result;
}


inline unsigned int lcm(std::vector<unsigned int> &numbers) {
  unsigned int n = numbers.size();
  if (n == 0) {
    return 0;
  }

  unsigned int result = numbers[0];
  for (unsigned int i = 1; i < n; i++) {
    result = ((numbers[i] * result) / (gcd(numbers[i], result)));
  }

  return result;
}


} // namespace util
} // namespace tribalinterfaces
