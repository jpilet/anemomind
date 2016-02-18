/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/grammars/WindOrientedGrammar.h>
#include <server/common/string.h>
#include <iostream>
#include <server/math/hmm/StateAssign.h>
#include <server/common/ArrayIO.h>
#include <server/common/HNodeGroup.h>
#include <server/common/logging.h>
#include <server/nautical/grammars/StaticCostFactory.h>
#include <server/nautical/grammars/HintedStateAssignFactory.h>
#include <server/common/SharedPtrUtils.h>


namespace sail {

WindOrientedGrammarSettings::WindOrientedGrammarSettings() {
/*
 * Default parameters chosen by hand.
 * We should devise a strategy to choose them.
 *
 */
  perSecondCost = 0.01;
  majorTransitionCost = 16.0;
  minorTransitionCost = 1.0;
  onOffCost = 2*majorTransitionCost;
  majorStateCost = 1.0;
  switchOnOffDuringRace = true;
}



namespace {
  // This is the number of __states__
  const int stateCount = 4*6 + 1;

  bool isValidState(int stateIndex) {
    return 0 <= stateIndex && stateIndex < stateCount;
  }


  const char *majorStates[] = {"before-race", "upwind-leg", "downwind-leg", "idle", "off"};
  const char *sides[] = {"starboard-tack", "port-tack"};
  const char *types[] = {"close-hauled", "beam-reach", "broad-reach"};

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

  HNodeGroup terminal(int index) {
    return HNodeGroup(index, getType(index % 6));
  }

  Hierarchy makeHierarchy() {

    HNodeGroup notInRace(37, "Not in race",
                        HNodeGroup(33, "before-race", // 25, 26
                            HNodeGroup(25, sides[0],
                                terminal(0) + terminal(1) + terminal(2)
                            )
                            +
                            HNodeGroup(26, sides[1],
                                terminal(3) + terminal(4) + terminal(5)
                            )
                        )
                        +
                        HNodeGroup(36, "idle", // 31, 32
                            HNodeGroup(31, sides[0],
                                terminal(18) + terminal(19) + terminal(20)
                            )
                            +
                            HNodeGroup(32, sides[1],
                                terminal(21) + terminal(22) + terminal(23)
                            )
                        )
                    );

    HNodeGroup inRace(38, "In race",
                        HNodeGroup(34, "upwind-leg", // 27, 28
                            HNodeGroup(27, sides[0],
                                terminal(6) + terminal(7) + terminal(8)
                            )
                            +
                            HNodeGroup(28, sides[1],
                                terminal(9) + terminal(10) + terminal(11)
                            )
                        )
                        +
                        HNodeGroup(35, "downwind-leg", // 29, 30
                            HNodeGroup(29, sides[0],
                                terminal(12) + terminal(13) + terminal(14)
                            )
                            +
                            HNodeGroup(30, sides[1],
                                terminal(15) + terminal(16) + terminal(17)
                            )
                        )
                    );

    HNodeGroup sailing(39, "Sailing", notInRace + inRace);

    HNodeGroup g(40, "Top", sailing + HNodeGroup(24, "Off"));

    return g.compile("Grammar001-%03d");
  }

  MDArray2b makeCon(const Hierarchy &h, int i, int j) {
    StaticCostFactory f(h);
    f.connectNoCost(i, j, false);
    return f.connections();
  }

  MDArray2b makeSOR(const Hierarchy &h) {
    return makeCon(h, h.getNodeByName("Not in race").index(), h.getNodeByName("In race").index());
  }

  MDArray2b makeEOR(const Hierarchy &h) {
    return makeCon(h, h.getNodeByName("In race").index(), h.getNodeByName("Not in race").index());
  }
}




WindOrientedGrammar::WindOrientedGrammar(WindOrientedGrammarSettings s) :
    _settings(s), _hierarchy(makeHierarchy()) {
    _startOfRaceTransitions = makeSOR(_hierarchy);
    _endOfRaceTransitions = makeEOR(_hierarchy);
}




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


  double getG001StateTransitionCost(const WindOrientedGrammarSettings &s,
      int from, int to, int at, NavCollection navs) {
    if (isOff(from) || isOff(to)) {
      return s.onOffCost*majorStateTransitionCost(from, to);
    } else {
      Duration<double> dur = navs[at+1].time() - navs[at].time();
      double seconds = dur.seconds();
      assert(seconds >= 0.0);
      return s.minorTransitionCost*minorStateTransitionCost(from, to) +
              s.majorTransitionCost*majorStateTransitionCost(from, to) +
              seconds*s.perSecondCost;
    }
  }


  Arrayd makeCostFactors() {
    const int major = 4;
    const int minor = 6;
    const int count = major*minor;
    const double expectedData[] = {1, 0, 1, 1, 0, 1, // before race

                                0, 1.0, 1, 1, 1.0, 0, // upwind leg

                                1, 1.0, 0, 0, 1.0, 1, // downwind leg

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
        int index = offs + j;
        factors[index] = f*expectedData[index];
      }
    }
    return factors;
  }



  int mapToRawMinorState(double twaDegs) {
    if (std::isnan(twaDegs)) {
      return -1;
    }
    double atMost360 = positiveMod(twaDegs, 360.0);
    return int(atMost360/60);
  }

  int mapToRawMinorState(const Nav &nav) {
    return mapToRawMinorState(
        toFinite(nav.twaFromTrueWindOverGround().degrees(),
            nav.externalTwa().degrees()));
  }
}


class G001SA : public StateAssign {
 public:
  G001SA(WindOrientedGrammarSettings s, NavCollection navs);

  double getStateCost(int stateIndex, int timeIndex);

  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);
  int getStateCount() {return stateCount;}
  int getLength() {return _navs.size();}

  Arrayi getPrecedingStates(int stateIndex, int timeIndex) {return _preds[stateIndex];}
 private:
  Array<Arrayi> _preds;
  WindOrientedGrammarSettings _settings;
  NavCollection _navs;
  Arrayd _minorStateCostFactors;
};

double G001SA::getStateCost(int stateIndex, int timeIndex) {
  Nav &nav = _navs[timeIndex];
  if (isOff(stateIndex)) {
    return _settings.majorStateCost;
  } else if (std::isnan(nav.awa().degrees())) {
    return _settings.majorStateCost;
  } else {
    int i0 = getMinorState(stateIndex);
    int i1 = mapToRawMinorState(nav);

    // Constant cost for being in this state
    double stateCost = _settings.majorStateCost*_minorStateCostFactors[stateIndex];

    // Penalty for this minor state index not matching the input
    double matchCost = (i0 == i1? 0 : 1);

    return stateCost + matchCost;
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

  MDArray2b makeConnections(bool switchOnOffDuringRace) {
    MDArray2b con(stateCount, stateCount);
    con.setAll(false);
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

    // The device can be switched on/off while
    // boat is idle.
    connectMajorStates(con, 3, 4);
      assert(isOff(24) && getMajorState(24) == 4);
    connectMajorStates(con, 4, 3);


    if (switchOnOffDuringRace) {
      for (int i = 0; i < 4; i++) {
        connectMajorStates(con, i, 4);
        connectMajorStates(con, 4, i);
      }
    }

    return con;
  }
}

G001SA::G001SA(WindOrientedGrammarSettings s, NavCollection navs) :
    _settings(s), _navs(navs), _minorStateCostFactors(makeCostFactors()),
    _preds(makePredecessorsPerState(makeConnections(s.switchOnOffDuringRace))) {
}

double G001SA::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
  return getG001StateTransitionCost(_settings, fromStateIndex, toStateIndex, fromTimeIndex, _navs);
}

std::shared_ptr<HTree> WindOrientedGrammar::parse(NavCollection navs,
    Array<UserHint> hints) {
  if (navs.empty()) {
    return std::shared_ptr<HTree>();
  }
  G001SA sa(_settings, navs);
  Arrayi states = makeHintedStateAssign(*this, makeSharedPtrToStack(sa), hints, navs).solve();
  return _hierarchy.parse(states);
}




//Grammar001::Grammar001(/*Grammar001Settings s*/) : /*_settings(s), */_hierarchy(makeHierarchy()) {}

} /* namespace sail */
