/*
 *  Created on: 2014-07-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CommonRaceGrammar.h"
#include <server/nautical/grammars/StaticCostFactory.h>
#include <server/common/HNodeGroup.h>
#include <server/math/hmm/StateAssign.h>
#include <server/common/SharedPtrUtils.h>
#include <server/nautical/grammars/HintedStateAssignFactory.h>

namespace sail {

using namespace NavCompat;

double OnOffCost::getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
  if (fromStateIndex == _offStateIndex || toStateIndex == _offStateIndex) {
    return 0;
  } else {
    Duration<double> dur = _navs[fromTimeIndex+1].time() - _navs[fromTimeIndex].time();
    double seconds = dur.seconds();
    assert(seconds >= 0.0);
    return seconds*_perSecondCost;
  }
}

CommonRaceGrammarSettings::CommonRaceGrammarSettings() {
  perSecondCost = 0.01;
  raceStartOrFinishCost = 16.0;
  sailPointTransitionCost = 0.5;
  onOffCost = 16;
  idleCost = 1.0;
  angleDifCost = 1.0;
  switchOnOffDuringRace = true;
}


namespace {
  Hierarchy makeH() {
    return HNodeGroup(12, "top",
             HNodeGroup(0, "off")
             +
             HNodeGroup(11, "sailing",
                 HNodeGroup(1, "idle")
                 +
                 HNodeGroup(10, "in-race",
                     HNodeGroup(8, "starboard-tack",
                         HNodeGroup(2, "close-hauled") + HNodeGroup(3, "beam-reach") + HNodeGroup(4, "broad-reach")
                     )
                     +
                     HNodeGroup(9, "port-tack",
                         HNodeGroup(5, "broad-reach") + HNodeGroup(6, "beam-reach") + HNodeGroup(7, "close-hauled")
                     )
                 )
             )
           ).compile("CommonRace-%03d");
  }

  const int terminalCount = 8;

  // States 2 to 7 represent sailing directions (broad-reach, beam-reach, close-hauled, on each side)
  double sailPointTransitionCost(int i, int j) {
    return cyclicDif(i-2, j-2, 6);
  }

  MDArray2b makeCon(const Hierarchy &h, int i, int j) {
    StaticCostFactory f(h);
    f.connectNoCost(i, j, false);
    return f.connections();
  }

  MDArray2b makeSOR(const Hierarchy &h) {
    return makeCon(h, h.getNodeByName("idle").index(), h.getNodeByName("in-race").index());
  }

  MDArray2b makeEOR(const Hierarchy &h) {
    return makeCon(h, h.getNodeByName("in-race").index(), h.getNodeByName("idle").index());
  }


}

CommonRaceGrammar::CommonRaceGrammar(CommonRaceGrammarSettings settings) :
  _h(makeH()) {
  _startOfRaceTransitions = makeSOR(_h);
  _endOfRaceTransitions = makeEOR(_h);
  double ooc = settings.onOffCost;
  StaticCostFactory f(_h);
  f.connectSelf(1, 1);
  f.connect(1, 10, settings.raceStartOrFinishCost, true);
  f.connect(0, 1, ooc, true);
  if (settings.switchOnOffDuringRace) {
    f.connect(10, 0, ooc, true);
  }
  f.addStateCost(1, settings.idleCost);
  f.connect(10, 10, [=](int i, int j) {
    return sailPointTransitionCost(i, j)*settings.sailPointTransitionCost;
  }, false);

  _staticTransitionCosts = f.staticTransitionCosts();
  _staticStateCosts = f.staticStateCosts();
  _preds = StateAssign::makePredecessorsPerState(f.connections());

  for (int i = 0; i < 6; i++) { // 30, 90, 150 210, 270, 330
    _angleCost.add(2 + i, Angle<double>::degrees(30 + 60*i));
  }
}

namespace {
  class CommonRaceStateAssign : public StateAssign {
   public:
    CommonRaceStateAssign(Array<Nav> navs, AngleCost &ac,
        MDArray2d staticTransitionCosts,
        Arrayd staticStateCosts, Array<Arrayi> preds, OnOffCost ooc) :
        _navs(navs), _angleCost(ac), _staticTransitionCosts(staticTransitionCosts),
        _staticStateCosts(staticStateCosts), _predecessors(preds), _onOffCost(ooc) {}

    double getStateCost(int stateIndex, int timeIndex) {
      return _staticStateCosts[stateIndex] + _angleCost.calcCost(stateIndex, _navs[timeIndex].externalTwa());
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return _staticTransitionCosts(fromStateIndex, toStateIndex) +
      _onOffCost.getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
    }

    int getStateCount() {
      return _staticStateCosts.size();
    }

    int getLength() {
      return _navs.size();
    }

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      return _predecessors[stateIndex];
    }
   private:
    Array<Nav> _navs;
    AngleCost _angleCost;
    MDArray2d _staticTransitionCosts;
    Arrayd _staticStateCosts;
    Array<Arrayi> _predecessors;
    OnOffCost _onOffCost;
  };
}

std::shared_ptr<HTree> CommonRaceGrammar::parse(NavDataset navs0,
    Array<UserHint> hints) {
  auto navs = makeArray(navs0);
  OnOffCost onOffCost(navs, 0, _settings.perSecondCost);
  CommonRaceStateAssign sa(navs, _angleCost, _staticTransitionCosts,
      _staticStateCosts, _preds, onOffCost);
  auto inds = makeHintedStateAssign(*this, makeSharedPtrToStack(sa), hints, navs).solve();
  return _h.parse(inds);
}

Array<HNode> CommonRaceGrammar::nodeInfo() const {
  return _h.nodes();
}

}
