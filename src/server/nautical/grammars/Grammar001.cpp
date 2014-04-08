/*
 *  Created on: 7 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/Grammar001.h>
#include <server/common/string.h>
#include <iostream>
#include <server/math/hmm/StateAssign.h>

namespace sail {

Grammar001Settings::Grammar001Settings() {
  _perSecondCost = 1.0;
  _majorTransitionCost = 2.0;
  _minorTransitionCost = 2.0;
  _onOffCost = 4.0;
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

  double minorStateTransitionCost(int from, int to) {
    int i = getMinorState(from);
    int j = getMinorState(to);
    return std::min(positiveMod(i - j, 6), positiveMod(j - i, 6));
  }

  double majorStateTransitionCost(int from, int to) {
    int i = getMajorState(from);
    int j = getMajorState(to);
    return (i == j? 0.0 : 1.0);
  }


  double getG001StateTransitionCost(const Grammar001Settings &s,
      int from, int to, int at, Array<Nav> navs) {
    if (isOff(from) || isOff(to)) {
      return s.onOffCost()*majorStateTransitionCost(from, to);
    } else {
      Duration<double> dur = navs[at+1].time() - navs[at].time();
      double seconds = dur.seconds();
      assert(seconds >= 0.0);
      return s.minorTransitionCost()*minorStateTransitionCost(from, to) +
              s.majorTransitionCost()*majorStateTransitionCost(from, to) +
              seconds*s.perSecondCost();
    }
  }


  Arrayd makeCostFactors() {
    const int major = 4;
    const int minor = 6;
    const int count = major*minor;
    const double expectedData[] = {1, 0, 1, 1, 0, 1, // before race

                                0, 0.5, 1, 1, 0.5, 0, // upwind leg

                                1, 0.5, 0, 0, 0.5, 1, // downwind leg

                                1, 1, 1, 1, 1, 1}; // idle
    Arrayd factors(count);
    for (int i = 0; i < major; i++) {
      int offs = i*minor;
      double s = 0.0;
      for (int j = 0; j < minor; j++) {
        s += expectedData[offs + j];
      }
      double f = 1.0/s;
      for (int j = 0; j < minor; j++) {
        factors[offs + j] = f*expectedData[offs + j];
      }
    }
    assert(std::abs(factors[count-1] - 1.0/6) < 1.0e-9);
    return factors;
  }

  int mapToRawMinorState(const Nav &nav) {
    return int(positiveMod(nav.awa().degrees(), 360.0)/60);
  }

}

std::shared_ptr<HTree> Grammar001::parse(Array<Nav> navs) {
  return std::shared_ptr<HTree>();
}

class G001SA : public StateAssign {
 public:
  G001SA(Grammar001Settings s, Array<Nav> navs);

  double getStateCost(int stateIndex, int timeIndex);

  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);
  int getStateCount() {return stateCount;}
  int getLength() {return _navs.size();}

  Arrayi getPrecedingStates(int stateIndex, int timeIndex);
 private:
  Array<Arrayi> _preds;
  Grammar001Settings _settings;
  Array<Nav> _navs;
  Arrayd _factors;
};

double G001SA::getStateCost(int stateIndex, int timeIndex) {
  Nav &n = _navs[timeIndex];
  if (isOff(stateIndex)) {
    return 0;
  } else {
    int i0 = getMinorState(stateIndex);
    int i1 = mapToRawMinorState(n);
    return _factors[stateIndex]*(i0 == i1? 0 : 1);
  }
}

namespace {
  const int minorPerMajor[5] = {6, 6, 6, 6, 1};

  Arrayi makeOffsets() {
    Arrayi offsets(5);
    int offs = 0;
    for (int i = 0; i < 5; i++) {
      offsets[i] = offs;
      offs += minorPerMajor[i];
    }
    return offsets;
  }

  Arrayi offsets = makeOffsets();

  void connectMajorStates(MDArray2b con, int I, int J) {
    for (int i = 0; i < minorPerMajor[I]; i++) {
      for (int j = 0; j < minorPerMajor[J]; j++) {
        con(i + offsets[I], j + offsets[J]) = true;
      }
    }
  }

  MDArray2b makeConnections() {
    MDArray2b con(stateCount, stateCount);
    for (int i = 0; i < 5; i++) {
      connectMajorStates(con, i, i);
    }
    connectMajorStates(con, 0, 1);
    connectMajorStates(con, 0, 2);
    connectMajorStates(con, 1, 2);
    connectMajorStates(con, 2, 1);
    connectMajorStates(con, 1, 3);
    connectMajorStates(con, 2, 3);
    connectMajorStates(con, 3, 0);

    connectMajorStates(con, 3, 4);
      assert(isOff(24) && getMajorState(24) == 4);
    connectMajorStates(con, 4, 3);

    return con;
  }
}

G001SA::G001SA(Grammar001Settings s, Array<Nav> navs) :
    _settings(s), _navs(navs), _factors(makeCostFactors()), _preds(makePredecessorsPerState(makeConnections())) {}

double G001SA::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
  return getG001StateTransitionCost(_settings, fromStateIndex, toStateIndex, fromTimeIndex, _navs);
}

//Grammar001::Grammar001(/*Grammar001Settings s*/) : /*_settings(s), */_hierarchy(makeHierarchy()) {}

} /* namespace sail */
