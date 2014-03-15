/*
 *  Created on: 14 mars 2014
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FilteredNavs.h"
#include <server/math/hmm/StateAssign.h>
#include <server/common/PeriodicHist.h>
#include <server/math/nonlinear/SignalFit.h>
#include <server/plot/extra.h>
#include <server/common/string.h>

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
  Arrayb rel = identifyReliableValues(stateCount, transitionCost, aws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();}),
      span);
  for (int i = 0; i < rel.size(); i++) {
    if (aws[i].metersPerSecond() == 0) {
      rel[i] = false;
    }
  }
  return rel;
}

Arrayb identifyReliablePeriodicValues(int stateCount, double transitionCost, Arrayi rawStates) {
  ValAssign assign(rawStates, stateCount, transitionCost, true);
  return compareStates(assign.solve(), rawStates);
}

Arrayb identifyReliableAwa(Array<Angle<double> > awa) {
  int stateCount = 12;

  PeriodicHistIndexer indexer(stateCount);
  double transitionCost = 2.0;
  Arrayb rel = identifyReliablePeriodicValues(stateCount, transitionCost,
      awa.map<int>([&](Angle<double> x) {return indexer.toBin(x);}));
  for (int i = 0; i < rel.size(); i++) {
    if (awa[i].radians() == 0) {
      rel[i] = false;
    }
  }
  return rel;
}



namespace {
  double floatMod(double a, double b) {
    if (a < 0) {
      return floatMod(a - (floor(a/b) - 3)*b, b);
    } else {
      return a - floor(a/b)*b;
    }
  }

  double pimod(double a) {
    double x = floatMod(a + M_PI, 2.0*M_PI);
    assert(0 <= x);
    assert(x < 2.0*M_PI);
    return x - M_PI;
  }

  Angle<double> makeContinuousAngle(Angle<double> a, Angle<double> b) {
    double difrad = (b - a).radians();
    return Angle<double>::radians(a.radians() + pimod(difrad));
  }
}

Array<Angle<double> > makeContinuousAngles(Array<Angle<double> > X) {
  int count = X.size();
  Array<Angle<double> > Y(count);
  Y[0] = X[0];
  for (int i = 1; i < count; i++) {
    Y[i] = makeContinuousAngle(Y[i-1], X[i]);
    assert(std::abs((Y[i] - Y[i-1]).radians()) <= M_PI);
  }
  return Y;
}

namespace {
  Arrayd timeToSeconds(Array<Duration<double> > time) {
    return time.map<double>([&] (Duration<double> t) {return t.seconds();});
  }

  Arrayd anglesToRadians(Array<Angle<double> > angles) {
    return angles.map<double>([&] (Angle<double> x) {return x.radians();});
  }
}


FilteredSignal filterAws(LineStrip strip, Array<Duration<double> > time,
    Array<Velocity<double> > aws) {
  Arrayb rel = identifyReliableAws(aws);
  double regs[2] = {0.52749, 1.69937};
  Arrayd Y = fitLineStrip(strip, Arrayd(2, regs), timeToSeconds(time).slice(rel),
      aws.map<double>([&] (Velocity<double> x) {return x.metersPerSecond();}).slice(rel));
  return FilteredSignal(strip, Y);
}

FilteredSignal filterAwa(LineStrip strip, Array<Duration<double> > time,
    Array<Angle<double> > awa) {
  Arrayb rel = identifyReliableAwa(awa);
  double regs[2] = {0.258247, 1.98186};

  Arrayd Y = fitLineStrip(strip, Arrayd(2, regs), timeToSeconds(time).slice(rel),
    anglesToRadians(makeContinuousAngles(awa.slice(rel))));
  return FilteredSignal(strip, Y);
}


double FilteredSignal::value(double x) {
  int I[2];
  double W[2];
  _strip.makeVertexLinearCombination(&x, I, W);
  return W[0]*_values[I[0]] + W[1]*_values[I[1]];
}

double FilteredSignal::derivative(double x) {
  int I[2];
  double W[2];
  _strip.makeVertexLinearCombination(&x, I, W);
  return (_values[I[1]] - _values[I[0]])/(_strip.getEq(0)(I[1]) - _strip.getEq(0)(I[0]));
}

void FilteredSignal::plot(GnuplotExtra &plot) {
  plot.set_style("lines");
  plot.plot_xy(X(), Y());
}

void FilteredSignal::plot() {
  GnuplotExtra p;
  plot(p);
  p.show();
}


FilteredNavs::FilteredNavs() {
  // TODO Auto-generated constructor stub

}

FilteredNavs::~FilteredNavs() {
  // TODO Auto-generated destructor stub
}

} /* namespace sail */
