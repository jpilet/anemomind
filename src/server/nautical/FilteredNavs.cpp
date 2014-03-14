/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FilteredNavs.h"
#include <server/math/hmm/StateAssign.h>

namespace sail {

namespace {
  class ValAssign : public StateAssign {
   public:
    ValAssign(Arrayi rawStates, int stateCount, double transitionCost) :
      _rawStates(rawStates),
      _stateCount(stateCount),
      _transitionCost(transitionCost) {
          _allInds = listStateInds();
      }

    double getStateCost(int stateIndex, int timeIndex) {
      return (stateIndex == _rawStates[timeIndex]? 0.0 : 1.0);
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      return std::abs(fromStateIndex - toStateIndex)*_transitionCost;
    }

    int getStateCount() {return _stateCount;}

    int getLength() {return _rawStates.size();}

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {return _allInds;}
   private:
    Arrayi _rawStates, _allInds;
    int _stateCount;
    double _transitionCost;
  };

}

Arrayb identifyReliableValues(int stateCount, double transitionCost, Arrayd values, Span span) {
  if (!span.initialized()) {
    span = Span(values);
  }
  LineKM smap(span.getMinv(), span.getMaxv(), 0.0, 0.999999*stateCount);
  Arrayi rawStates = values.map<int>([&] (double x) {return int(floor(smap(x)));});
  ValAssign assign(rawStates, stateCount, transitionCost);
  Arrayi fitStates = assign.solve();
  int count = values.size();
  Arrayb reliable(count);
  for (int i = 0; i < count; i++) {
    reliable[i] = rawStates[i] == fitStates[i];
  }
  return reliable;
}

Arrayb identifyReliableAws(Array<Velocity<double> > aws) {
  Velocity<double> maxvel = Velocity<double>::metersPerSecond(30.0);
  Span span(0.0, maxvel.metersPerSecond());
  int stateCount = 60;
  double transitionCost = 4.0;
  return identifyReliableValues(stateCount, transitionCost, aws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();}),
      span);
}

FilteredNavs::FilteredNavs() {
  // TODO Auto-generated constructor stub

}

FilteredNavs::~FilteredNavs() {
  // TODO Auto-generated destructor stub
}

} /* namespace sail */
