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
#include <server/common/ArrayIO.h>

namespace sail {

namespace {
  class ValAssign : public StateAssign {
   public:
    ValAssign(Arrayi rawStates, int stateCount, double transitionCost, bool cyclic, int maxStep) :
      _rawStates(rawStates),
      _stateCount(stateCount),
      _transitionCost(transitionCost),
      _maxStep(maxStep),
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
      if (dist > _maxStep && _maxStep > 0) {
        return 1.0e6;
      }
      double cost = dist*_transitionCost;
      assert(cost > 0 || toStateIndex == fromStateIndex);
      return cost;
    }

    int getStateCount() {return _stateCount;}

    int getLength() {return _rawStates.size();}

    Arrayi getPrecedingStates(int stateIndex, int timeIndex) {return _allInds;}

   private:
    bool _cyclic;
    int _maxStep;
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


Arrayb identifyReliableValues(int stateCount, double transitionCost, Arrayd values, Spand span, int maxStep = -1) {
  if (!span.initialized()) {
    span = Spand(values);
  }
  LineKM smap(span.minv(), span.maxv(), 0.0, 0.999999*stateCount);
  Arrayi rawStates = values.map<int>([&] (double x) {return int(floor(smap(x)));});
  ValAssign assign(rawStates, stateCount, transitionCost, false, maxStep);
  Arrayi assigned = assign.solve();
  return compareStates(rawStates, assigned);
}

Arrayb identifyReliableAws(Array<Velocity<double> > aws) {
  Velocity<double> maxvel = Velocity<double>::metersPerSecond(30.0);
  Spand span(0.0, maxvel.metersPerSecond());
  int stateCount = 60;
  double transitionCost = 2.0;
  Arrayb rel = identifyReliableValues(stateCount, transitionCost, aws.map<double>([&](Velocity<double> x) {return x.metersPerSecond();}),
      span, -1);
  for (int i = 0; i < rel.size(); i++) {
    if (aws[i].metersPerSecond() == 0) {
      rel[i] = false;
    }
  }
  return rel;
}

Arrayb identifyReliablePeriodicValues(int stateCount, double transitionCost, Arrayi rawStates) {
  ValAssign assign(rawStates, stateCount, transitionCost, true, -1);
  return compareStates(assign.solve(), rawStates);
}


  int stateCount = 12;

Arrayb identifyReliableAngles(Array<Angle<double> > awa, double transitionCost) {
  PeriodicHistIndexer indexer(stateCount);
  Arrayb rel = identifyReliablePeriodicValues(stateCount, transitionCost,
      awa.map<int>([&](Angle<double> x) {return indexer.toBin(x);}));
  for (int i = 0; i < rel.size(); i++) {
    if (awa[i].radians() == 0) {
      rel[i] = false;
    }
  }
  return rel;
}

Arrayb identifyReliableAwa(Array<Angle<double> > awa) {
  return identifyReliableAngles(awa, 2.0);
}

Arrayb identifyReliableMagHdg(Array<Angle<double> > mh) {
  return identifyReliableAngles(mh, 2.0);
}

Arrayb identifyReliableGpsBearing(Array<Angle<double> > gb) {
  return identifyReliableAngles(gb, 2.0);
}

Arrayb identifyReliableWatSpeed(Array<Velocity<double> > ws) {
  Velocity<double> maxvel = Velocity<double>::knots(50.0);
  Spand span(0.0, maxvel.metersPerSecond());
  int stateCount = 50;
  double transitionCost = 2.0;
  Arrayb rel = identifyReliableValues(stateCount, transitionCost, ws.map<double>([&](Velocity<double> x) {return x.knots();}),
      span, -1);
  return rel;
}

Arrayb identifyReliableGpsSpeed(Array<Velocity<double> > ws) {
  Velocity<double> maxvel = Velocity<double>::knots(50.0);
  Spand span(0.0, maxvel.metersPerSecond());
  int stateCount = 50;
  double transitionCost = 8.0;
  Arrayb rel = identifyReliableValues(stateCount, transitionCost, ws.map<double>([&](Velocity<double> x) {return x.knots();}),
      span, -1);
  return rel;
}



namespace {

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

FilteredSignal filterMagHdg(LineStrip strip, Array<Duration<double> > T, Array<Angle<double> > maghdg) {
   Arrayb rel = identifyReliableMagHdg(maghdg);
   double regs[2] = {1.85308e-06, 2.2046};

   Arrayd Y = fitLineStrip(strip, Arrayd(2, regs), timeToSeconds(T).slice(rel),
     anglesToRadians(makeContinuousAngles(maghdg.slice(rel))));
   return FilteredSignal(strip, Y);
}

FilteredSignal filterGpsBearing(LineStrip strip, Array<Duration<double> > T, Array<Angle<double> > gb) {
  {
    Arrayd allCoords = strip.getGridVertexCoords1d();
    assert(allCoords.first() <= T.first().seconds());
    assert(T.last().seconds() <= allCoords.last());
  }
   Arrayb rel = identifyReliableGpsBearing(gb);

   int orders[1] = {2};
   double regs[1] = {1.33828};

   Arrayd Y = fitLineStrip(strip, Arrayi(1, orders), Arrayd(1, regs), timeToSeconds(T).slice(rel),
     anglesToRadians(makeContinuousAngles(gb.slice(rel))));
   return FilteredSignal(strip, Y);
}

FilteredSignal filterWatSpeed(LineStrip strip, Array<Duration<double> > T, Array<Velocity<double> > ws) {
  Arrayb rel = identifyReliableWatSpeed(ws);
  double regs[2] = {0.530101, 2.42038};
  Arrayd Y = fitLineStrip(strip, Arrayd(2, regs), timeToSeconds(T).slice(rel),
      ws.map<double>([&] (Velocity<double> x) {return x.metersPerSecond();}).slice(rel));
  return FilteredSignal(strip, Y);
}

FilteredSignal filterGpsSpeed(LineStrip strip, Array<Duration<double> > T, Array<Velocity<double> > gs) {
  Arrayb rel = identifyReliableGpsSpeed(gs);
  double regs[2] = {0.530101, 2.42038};
  Arrayd Y = fitLineStrip(strip, Arrayd(2, regs), timeToSeconds(T).slice(rel),
      gs.map<double>([&] (Velocity<double> x) {return x.metersPerSecond();}).slice(rel));
  return FilteredSignal(strip, Y);
}


LineStrip makeNavsLineStrip(Array<Duration<double> > T) {
  Arrayd X = timeToSeconds(T);
  Spand exp = Spand(X).expand(0.1);
  std::cout << EXPR_AND_VAL_AS_STRING(exp) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(T.first().seconds()) << std::endl;
  std::cout << EXPR_AND_VAL_AS_STRING(X.first()) << std::endl;
  return LineStrip(exp, 1.0);
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


FilteredNavs::FilteredNavs(Array<Nav> navs) {
  times = getLocalTime(navs);
  time = makeNavsLineStrip(times);
  {
    Arrayd allCoords = time.getGridVertexCoords1d();
    assert(allCoords.first() <= times.first().seconds());
    assert(times.last().seconds() <= allCoords.last());
  }

  aws = filterAws(time, times, getAws(navs));
  awa = filterAwa(time, times, getAwa(navs));
  magHdg = filterMagHdg(time, times, getMagHdg(navs));
  gpsBearing = filterGpsBearing(time, times, getGpsBearing(navs));
  watSpeed = filterWatSpeed(time, times, getWatSpeed(navs));
  gpsSpeed = filterGpsSpeed(time, times, getGpsSpeed(navs));
}


} /* namespace sail */
