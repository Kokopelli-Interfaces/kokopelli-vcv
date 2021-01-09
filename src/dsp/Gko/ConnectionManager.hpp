#pragma once

#include "definitions.hpp"
#include "Layer.hpp"
#include <string>

#include <memory>

namespace myrisa {
namespace dsp {
namespace gko {

class ConnectionManager {
public:
  // TODO shared pointer?
  std::vector<std::shared_pointer<Connection>> connections;
  std::vector<std::shared_pointer<Connection>> active_connections;

public:
  inline void addConnection(std::shared_pointer<Connection> connection) {
    connections.push_back(connection);
  }

  inline void refreshActiveConnections() {
    active_connections.clear();
    for (auto connection : connections) {
      if (connection->active) {
        active_connections.push_back(connection);
      }
    }
  }
};

} // namespace gko
} // namespace dsp
} // namepsace myrisa
