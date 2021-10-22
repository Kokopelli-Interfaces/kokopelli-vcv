#pragma once

#include "definitions.hpp"
#include "EmernetNodeGroup.hpp"
#include "EmernetNode.hpp"
#include "LiveNode.hpp"
#include "dsp/PhaseDivider.hpp"

namespace kokopellivcv {
namespace emernet {

struct EmernetTraverser {
  static inline EmernetNode getNode(Emernet* emernet, emernet_node_id node_id) {
    EmernetNodeGroup* next_group = emernet;
    for (unsigned int i = 0; i < node_id.size() - 1; i++) {
      unsigned int node_i = node_id[i];
      next_group = next_group->_nodes[node_i].group;
    }

    unsigned int node_i = node_id[node_id.size()-1];
    return next_group->_nodes[node_i];
  }

  static inline LiveNode* getLiveNode(Emernet* emernet, emernet_node_id node_id) {
    return getNode(emernet, node_id).liveNode;
  }

  static inline EmernetNodeGroup* getGroup(Emernet* emernet, emernet_node_id node_id) {
    return getNode(emernet, node_id).group;
  }

  static inline EmernetNodeGroup* getNodesGroup(Emernet* emernet, emernet_node_id id_of_node_in_group) {
    if (id_of_node_in_group.size() == 1) {
      return emernet;
    } else {
      emernet_node_id group_emernet_node_id = id_of_node_in_group;
      group_emernet_node_id.pop_back();
      return getNode(emernet, group_emernet_node_id).group;
    }
  }

  static inline emernet_node_id putNodeIntoBaseGroup(Emernet* emernet, Node node) {
    emernet->addNode(node);
    emernet_node_id new_emernet_node_id;
    new_emernet_node_id.push_back(emernet->getNumberOfNodes()-1);
    return new_emernet_node_id;
  }

  static inline emernet_node_id putNodeIntoSubGroup(Emernet* emernet, Node node, emernet_node_id group_id) {
    EmernetNodeGroup* group = getNode(emernet, group_id).group;
    group->addNode(node);

    emernet_node_id new_node_id = group_id;
    unsigned int node_i_in_group = group->getNumberOfNodes() - 1;
    new_node_id.push_back(node_i_in_group);
    return new_node_id;
  }

  static inline emernet_node_id putNodeIntoNodesGroup(Emernet* emernet, Node node, emernet_node_id node_in_group) {
    EmernetNodeGroup* group = getNodesGroup(emernet, node_in_group);
    group->addNode(node);
    emernet_node_id new_node_id = node_in_group;
    new_node_id.back() = group->getNumberOfNodes()-1;
    return new_node_id;
  }
};

} // namespace emernet
} // namespace kokopellivcv
