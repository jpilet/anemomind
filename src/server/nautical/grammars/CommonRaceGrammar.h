/*
 *  Created on: Jul 2, 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef COMMONRACEGRAMMAR_H_
#define COMMONRACEGRAMMAR_H_

#include <cmath>
#include <cassert>
#include <server/nautical/grammars/Grammar.h>
#include <server/nautical/grammars/AngleCost.h>

namespace sail {

class OnOffCost {
 public:
  OnOffCost() : _offStateIndex(-1), _perSecondCost(NAN) {}
  OnOffCost(NavCollection navs, int offStateIndex,
      double perSecondCost) : _navs(navs),
          _offStateIndex(offStateIndex),
          _perSecondCost(perSecondCost) {}

  double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex);
 private:
  NavCollection _navs;
  int _offStateIndex;
  double _perSecondCost;
};

class CommonRaceGrammarSettings {
 public:
  CommonRaceGrammarSettings();

  double perSecondCost;   // To encourage the device to be turned off when it is.
  double angleDifCost;    // Cost for angle deviation when racing
  double onOffCost;       // Cost for switching on/off the device
  double raceStartOrFinishCost; // Cost for switching between major states
  double sailPointTransitionCost; // Cost for switching between minor states
  double idleCost;

  bool switchOnOffDuringRace;
};

class CommonRaceGrammar : public Grammar {
 public:
  CommonRaceGrammar(CommonRaceGrammarSettings settings);

  std::shared_ptr<HTree> parse(NavCollection navs,
      Array<UserHint> hints = Array<UserHint>());
  virtual Array<HNode> nodeInfo() const;
  MDArray2b startOfRaceTransitions() const {return _startOfRaceTransitions;}
  MDArray2b endOfRaceTransitions() const {return _endOfRaceTransitions;}
 private:
  MDArray2b _startOfRaceTransitions, _endOfRaceTransitions;
  AngleCost _angleCost;
  CommonRaceGrammarSettings _settings;
  Hierarchy _h;
  Array<Arrayi> _preds;
  MDArray2d _staticTransitionCosts;
  Arrayd _staticStateCosts;
};

}

#endif /* COMMONRACEGRAMMAR_H_ */
