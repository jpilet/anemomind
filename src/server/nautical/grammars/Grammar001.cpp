/*
 *  Created on: 7 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/Grammar001.h>
#include <server/common/string.h>

namespace sail {

//Grammar001Settings::Grammar001Settings() {
//  _majorTransitionCost = 2.0;
//  _minorTransitionCost = 2.0;
//}



namespace {
  typedef const char *Str;

  // This is the number of __states__
  const int stateCount = 36 + 1;

  bool isValidState(int stateIndex) {
    return 0 <= stateIndex && stateIndex < stateCount;
  }


  Str majorStates[] = {"before-race", "upwind-leg", "downwind-leg", "idle", "off"};
  Str sides[] = {"starboard-tack", "port-tack"};
  Str types[] = {"close-hauled", "beam-reach", "broad-reach"};

  const char *getType(int minorIndex) {
    const int typeMap[6] = {0, 1, 2, 2, 1, 0};
    return types[typeMap[minorIndex]];
  }


  std::string getMinorStateLabel(int minorIndex) {
    int side = (minorIndex < 3? 0 : 1);
    return std::string(sides[side]) + "-" + getType(minorIndex);
  }

  std::string getStateLabel(int stateIndex) {
    const int minorStatesPerMajorState = 6;
    int majorIndex = stateIndex/minorStatesPerMajorState;
    if (majorIndex == 4) {
      return std::string(majorStates[majorIndex]);
    } else {
      int minorIndex = stateIndex - minorStatesPerMajorState*majorIndex;
      return std::string(majorStates[majorIndex]) + "-" + getMinorStateLabel(minorIndex);
    }
  }

  std::string makeNodeCode(int nodeIndex) {
    return stringFormat("grammar001-%03d", nodeIndex);
  }

  Hierarchy makeHierarchy() {
    std::vector<HNode> nodes;

    // 0..35
    // 37,38 39,40 41,42 43,44
    // 45    46    47    48
    // 49    50    50    49
    //          51

    const int majorParents[4] = {49, 50, 50, 49};
    for (int i = 0; i < 4; i++) {// every major state
      int majorIndex = 45 + i;
      nodes.push_back(HNode(majorIndex, majorParents[i], majorStates[i]));
      for (int j = 0; j < 2; j++) { // Starboard/Port?
        int sideIndex = 37+j + 2*i;
        nodes.push_back(HNode(sideIndex, majorIndex, sides[j]));
        for (int k = 0; k < 3; k++) { // type, close-hauled, beam reach or broad reach?
          int minorIndex = 3*j + k;
          int stateIndex = 6*i + minorIndex;
          nodes.push_back(HNode(stateIndex, sideIndex, getType(minorIndex)));
        }
      }
    }
    nodes.push_back(HNode(49, 51, "Not in race"));
    nodes.push_back(HNode(50, 51, "In race"));
    nodes.push_back(HNode(51, 52, "Sailing"));
    nodes.push_back(HNode(36, 52, "Off"));
    nodes.push_back(HNode::makeRoot(52, "Top"));
    return Hierarchy(Array<HNode>::referToVector(nodes).dup());
  }
}

std::shared_ptr<HTree> Grammar001::parse(Array<Nav> navs) {
  return std::shared_ptr<HTree>();
}

Array<GrammarNodeInfo> Grammar001::nodeInfo() {
  return Array<GrammarNodeInfo>::fill(stateCount, [&] (int index) {
    return GrammarNodeInfo(makeNodeCode(index), _hierarchy.node(index).label());
  });
}

Grammar001::Grammar001(/*Grammar001Settings s*/) : /*_settings(s), */_hierarchy(makeHierarchy()) {}

} /* namespace sail */
