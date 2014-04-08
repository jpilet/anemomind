/*
 *  Created on: 7 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/Grammar001.h>
#include <server/common/string.h>
#include <iostream>

namespace sail {

Grammar001Settings::Grammar001Settings() {
  _majorTransitionCost = 2.0;
  _minorTransitionCost = 2.0;
}



namespace {
  typedef const char *Str;

  // This is the number of __states__
  const int stateCount = 4*6 + 1;

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

    // 0..24
    // 25,26 27,28 29,30 31,32
    // 33    34    35    36
    // 37    38    38    37
    //

    int mcounter = 0;
    const int majorParents[4] = {37, 38, 38, 37};
    for (int i = 0; i < 4; i++) {// every major state
      int majorIndex = 33 + i;
      nodes.push_back(HNode(majorIndex, majorParents[i], majorStates[i]));
      for (int j = 0; j < 2; j++) { // Starboard/Port?
        int sideIndex = 25+j + 2*i;
        nodes.push_back(HNode(sideIndex, majorIndex, sides[j]));
        for (int k = 0; k < 3; k++) { // type, close-hauled, beam reach or broad reach?
          int minorIndex = 3*j + k;
          int stateIndex = 6*i + minorIndex;
          nodes.push_back(HNode(stateIndex, sideIndex, getType(minorIndex)));
          mcounter++;
        }
      }
    }
    nodes.push_back(HNode(37, 39, "Not in race"));
    nodes.push_back(HNode(38, 39, "In race"));
    nodes.push_back(HNode(39, 40, "Sailing"));
    nodes.push_back(HNode(24, 40, "Off"));
    nodes.push_back(HNode::makeRoot(40, "Top"));
    return Hierarchy(Array<HNode>::referToVector(nodes).dup());
  }
}



Grammar001::Grammar001(Grammar001Settings s) : _settings(s), _hierarchy(makeHierarchy()) {}




namespace {
  int getMajorState(int stateIndex) {return stateIndex/6;}
  int getMinorState(int stateIndex) {return stateIndex % 6;}
  const int offState = 24;
  bool isOff(int stateIndex) {return stateIndex == offState;}

  int minorStateTransitionCost(int from, int to) {

  }



  double stateTransitionCost(int from, int to, int at, Array<Nav> navs) {
    if (isOff(from)) {

    } else if (isOff(to)) {

    } else {

    }
  }
}

std::shared_ptr<HTree> Grammar001::parse(Array<Nav> navs) {
  return std::shared_ptr<HTree>();
}

//Grammar001::Grammar001(/*Grammar001Settings s*/) : /*_settings(s), */_hierarchy(makeHierarchy()) {}

} /* namespace sail */
