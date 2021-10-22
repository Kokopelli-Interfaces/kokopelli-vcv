#pragma once

namespace kokopellivcv {
namespace emernet {


// TODO create OSC->Conductor bridge
// TODO create Modules->Conductor bridge

  /*
   * The EmernetConductor focuses nodes in the emernet, and may adjust them. It's focused node is the light green square in the diagram. It executes all the user functions on the emernet.
   */
struct EmernetConductor {
  emernet_node_id _focused_node;
  float _love = 0.f;

  void loop();
  void loopLongPress();
  void next();
  void prev();

  void skipToActiveMember();

  void undo();
};

} // namespace emernet
} // namespace kokopellivcv
