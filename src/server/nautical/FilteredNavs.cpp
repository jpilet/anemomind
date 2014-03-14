/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FilteredNavs.h"
#include <server/math/hmm/StateAssign.h>
#include <server/common/PeriodicHist.h>

namespace sail {

namespace {
  class ValAssign : public StateAssign {
   public:
    ValAssign(Arrayi rawStates, int stateCount, double transitionCost, bool cyclic = false) :
      _rawStates(rawStates),
      _stateCount(stateCount),
      _transitionCost(transitionCost),
      _cyclic(cyclic) {
          _allInds = listStateInds();
      }

    double getStateCost(int stateIndex, int timeIndex) {
      return (stateIndex == _rawStates[timeIndex]? 0.0 : 1.0);
    }

    double getTransitionCost(int fromStateIndex, int toStateIndex, int fromTimeIndex) {
      int dist = 0;
      if (_cyclic) {
        int moddist = posMod(fromStateIndex - toStateIndex, _stateCount);
        dist = std::min(moddist, _stateCount - moddist);
      } else {
        dist = std::abs(fromStateIndex - toStateIndex);
      }
      return dist*_transitionCost;
    }

    int getStateCount() {return _stateCount;}

    int getLength() {return _rawStates.size();}

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {return _allInds;}

   private:
    bool _cyclic;
    Arrayi _rawStates, _allInds;
    int _stateCount;
    double _transitionCost;
  };

}

Arrayb compareStates(Arrayi a, Arrayi b) {
  int count = a.size();
  Arrayb reliable(count);
  for (int i = 0; i < count; i++) {
    reliable[i] = a[i] == b[i];
  }
  return reliable;
}


Arrayb identifyReliableValues(int stateCount, double transitionCost, Arrayd values, Span span) {
  if (!span.initialized()) {
    span = Span(values);
  }
  LineKM smap(span.getMinv(), span.getMaxv(), 0.0, 0.999999*stateCount);
  Arrayi rawStates = values.map<int>([&] (double x) {return int(floor(smap(x)));});
  ValAssign assign(rawStates, stateCount, transitionCost);
  return compareStates(rawStates, assign.solve());
}

Arrayb identifyReliableAws(Array<Velocity<double> > aws) {
  Velocity<double> maxvel = Velocity<double>::metersPerSecond(30.0);
  Span span(0.0, maxvel.metersPerSecond());
  int stateCount = 60;
  double transitionCost = 2.0;
  return identifyReliableValues(stateCount, transitionCost, aws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();}),
      span);
}

Arrayb identifyReliablePeriodicValues(int stateCount, double transitionCost, Arrayi rawStates) {
  ValAssign assign(rawStates, stateCount, transitionCost, true);
  return compareStates(assign.solve(), rawStates);
}

Arrayb identifyReliableAwa(Array<Angle<double> > awa) {
  int stateCount = 12;

  PeriodicHistIndexer indexer(stateCount);
  double transitionCost = 2.0;
  return identifyReliablePeriodicValues(stateCount, transitionCost,
      awa.map<int>([&](Angle<double> x) {return indexer.toBin(x);}));
}

FilteredNavs::FilteredNavs() {
  // TODO Auto-generated constructor stub

}

FilteredNavs::~FilteredNavs() {
  // TODO Auto-generated destructor stub
}

} /* namespace sail */
