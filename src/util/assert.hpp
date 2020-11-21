#include <iostream>

#define ASSERT(left, operator, right)                                                              \
  {                                                                                                \
    if (!((left) operator(right))) {                                                               \
      std::cerr                                                                                     \
          << "Myrisa Modules:: ASSERT FAILED: " << #left                                                            \
          << #                                                                                     \
             operator<< #right << " @ " << __FILE__ << " (" << __LINE__ << ")"                     \
                                                                           "."                     \
                                                                           " " << #left << "=" <<( \
                                                                               left)               \
          << "; " << #right << "=" << (right) << std::endl;                                        \
      exit(1);                                                                                \
    }                                                                                              \
  }
