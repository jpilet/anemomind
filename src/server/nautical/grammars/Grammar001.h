/*
 *  Created on: 2014-04-07
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRAMMAR001_H_
#define GRAMMAR001_H_

#include <server/common/Hierarchy.h>
#include <server/nautical/grammars/Grammar.h>


namespace sail {

struct Grammar001Settings {
  Grammar001Settings();

  double majorTransitionCost; // Cost to move between major states.
  double minorTransitionCost; // Cost to move between minor states
  double perSecondCost;       // Cost paid per second when device is turned on. This will encourage the device to
                              //  be turned off when there is a lot of time between measurements
  double onOffCost;           // cost for being in the off-state
  double majorStateCost;
  bool switchOnOffDuringRace;
};

class Grammar001 : public Grammar {
 public:
  Grammar001(Grammar001Settings s);
  std::shared_ptr<HTree> parse(Array<Nav> navs) ;
  Array<HNode> nodeInfo() {return _hierarchy.nodes();}
  virtual ~Grammar001() {}
 private:
  Hierarchy _hierarchy;
  Grammar001Settings _settings;
};

} /* namespace sail */

#endif /* GRAMMAR001_H_ */
