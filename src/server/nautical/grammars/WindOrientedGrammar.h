/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef WIND_ORIENTED_GRAMMAR_H_
#define WIND_ORIENTED_GRAMMAR_H_

#include <server/common/Hierarchy.h>
#include <server/nautical/grammars/Grammar.h>


namespace sail {


struct WindOrientedGrammarSettings {
  WindOrientedGrammarSettings();

  double majorTransitionCost; // Cost to move between major states.
  double minorTransitionCost; // Cost to move between minor states
  double perSecondCost;       // Cost paid per second when device is turned on. This will encourage the device to
                              //  be turned off when there is a lot of time between measurements
  double onOffCost;           // cost for being in the off-state
  double majorStateCost;
  bool switchOnOffDuringRace;
};

class WindOrientedGrammar : public Grammar {
 public:
  WindOrientedGrammar(WindOrientedGrammarSettings s);
  std::shared_ptr<HTree> parse(Array<Nav> navs,
      Array<UserHint> hints = Array<UserHint>()) ;
  Array<HNode> nodeInfo() {return _hierarchy.nodes();}
 private:
  Hierarchy _hierarchy;
  WindOrientedGrammarSettings _settings;
};

} /* namespace sail */

#endif  // WIND_ORIENTED_GRAMMAR_H_
