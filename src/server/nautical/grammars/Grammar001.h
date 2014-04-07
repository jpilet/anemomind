/*
 *  Created on: 7 avr. 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef GRAMMAR001_H_
#define GRAMMAR001_H_

#include <server/common/Hierarchy.h>
#include <server/nautical/grammars/Grammar.h>


namespace sail {

//class Grammar001Settings {
// public:
//  Grammar001Settings();
//  double majorTransitionCost() {return _majorTransitionCost;}
//  double minorTransitionCost() {return _minorTransitionCost;}
// private:
//  double _majorTransitionCost, _minorTransitionCost;
//};

class Grammar001 : public Grammar {
 public:
  Grammar001(); //Grammar001Settings s);
  std::shared_ptr<HTree> parse(Array<Nav> navs) ;
  Array<GrammarNodeInfo> nodeInfo();
  virtual ~Grammar001() {}
 private:
  Hierarchy _hierarchy;
  //Grammar001Settings _settings;
};

} /* namespace sail */

#endif /* GRAMMAR001_H_ */
