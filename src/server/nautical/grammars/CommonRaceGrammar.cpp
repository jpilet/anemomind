/*
 *  Created on: 2014-07-02
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "CommonRaceGrammar.h"
#include <server/nautical/grammars/StaticCostFactory.h>
#include <server/common/HNodeGroup.h>
#include <server/math/hmm/StateAssign.h>

namespace sail {

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
  majorTransitionCost = 16.0;
  minorTransitionCost = 1.0;
  onOffCost = 2*majorTransitionCost;
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

  double minorTransitionCost(int i, int j) {
    return cyclicDif(i-2, j-2, 6);
  }
}

CommonRaceGrammar::CommonRaceGrammar(CommonRaceGrammarSettings settings) :
  _h(makeH()) {
  double ooc = settings.onOffCost*settings.majorTransitionCost;
  StaticCostFactory f(_h);
  f.connect(1, 10, settings.majorTransitionCost, true);
  f.connect(0, 1, ooc, true);
  if (settings.switchOnOffDuringRace) {
    f.connect(10, 0, ooc, true);
  }
  f.addStateCost(1, settings.idleCost);
  f.connect(10, 10, [=](int i, int j) {
    return minorTransitionCost(i, j)*settings.minorTransitionCost;
  }, false);

  _staticTransitionCosts = f.staticTransitionCosts();
  _staticStateCosts = f.staticStateCosts();
  _preds = StateAssign::makePredecessorsPerState(f.connections());

  for (int i = 0; i < 6; i++) { // 30, 90, 150 210, 270, 330
    _angleCost.add(2 + i, Angle<double>::degrees(30 + 60*i));
  }
}

namespace {
  class SA : public StateAssign {
   public:
    SA(Array<Nav> navs, AngleCost &ac,
        MDArray2d staticTransitionCosts,
        Arrayd staticStateCosts, Array<Arrayi> preds, OnOffCost ooc) :
        _navs(navs), _ac(ac), _staticTransitionCosts(staticTransitionCosts),
        _staticStateCosts(staticStateCosts), _preds(preds), _ooc(ooc) {}

    double getStateCost(int stateIndex, int timeIndex) {
      return _staticStateCosts[stateIndex] + _ac.calcCost(stateIndex, _navs[timeIndex].externalTwa());
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return _staticTransitionCosts(fromStateIndex, toStateIndex) +
      _ooc.getTransitionCost(fromStateIndex, toStateIndex, fromTimeIndex);
    }

    int getStateCount() {
      return _staticStateCosts.size();
    }

    int getLength() {
      return _navs.size();
    }

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {
      return _preds[stateIndex];
    }
   private:
    Array<Nav> _navs;
    AngleCost _ac;
    MDArray2d _staticTransitionCosts;
    Arrayd _staticStateCosts;
    Array<Arrayi> _preds;
    OnOffCost _ooc;
  };
}

std::shared_ptr<HTree> CommonRaceGrammar::parse(Array<Nav> navs,
    Array<UserHint> hints) {
  OnOffCost onOffCost(navs, 0, _settings.perSecondCost);
  SA sa(navs, _angleCost, _staticTransitionCosts,
      _staticStateCosts, _preds, onOffCost);
  return _h.parse(sa.solve());
}

Array<HNode> CommonRaceGrammar::nodeInfo() {
  return _h.nodes();
}

}
