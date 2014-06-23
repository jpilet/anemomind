/*
 *  Created on: 2014-06-20
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 *
 *  Helper object to define the static transition costs and state costs
 *  for terminals in a grammar.
 */

#ifndef STATECONNECTOR_H_
#define STATECONNECTOR_H_

#include <server/common/MDArray.h>

namespace sail {

class Hierarchy;

class StaticCostFactory {
 public:
  StaticCostFactory(const Hierarchy &h);

  // Methods to retrieve the defined values so far
  MDArray2d staticTransitionCosts() const {return _transitionCosts;}
  Arrayd staticStateCosts() const {return _stateCosts;}
  MDArray2b connections() const {return _con;}



  // Methods to define various costs and connections
  void connect(int iSrc, int jDst, std::function<double(int,int)> f, bool symmetric = false);
  void connect(int iSrc, int jDst, double cost, bool symmetric = false);

  void addStateCost(int i, std::function<double(int)> f);
  void addStateCost(int i, double cost);
 private:
  Array<Arrayi> _terminalsPerNode;

  Arrayd _stateCosts;
  MDArray2d _transitionCosts;
  MDArray2b _con;
};

} /* namespace sail */

#endif /* STATECONNECTOR_H_ */
