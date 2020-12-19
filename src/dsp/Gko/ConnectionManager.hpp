#pragma once

#include "definitions.hpp"
#include "Layer.hpp"
#include "Channel.hpp"
#include <string>

namespace myrisa {
namespace dsp {
namespace gko {

class ConnectionManager {
  std::unordered_map<std::string, Connection*> channels;

  inline void addChannel(Channel* channel) {
    return;
  }
};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
