/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "FilteredNavData.h"
#include <algorithm>
#include <server/common/logging.h>
#include <server/math/CleanNumArray.h>
#include <server/math/nonlinear/GeneralizedTV.h>
#include <server/plot/extra.h>
#include <server/common/ScopedLog.h>
#include <server/common/string.h>
#include <server/common/MeanAndVar.h>
#include <server/common/Span.h>

namespace sail {

namespace {
  bool sameBoat(Array<Nav> navs) {
    Nav::Id boatId = navs[0].boatId();
    int count = navs.size();
    for (int i = 1; i < count; i++) {
      if (boatId != navs[i].boatId()) {
        return false;
      }
    }
    return true;
  }

  Array<Duration<double> > getTimes(Array<Nav> navs, TimeStamp offset) {
    return navs.map<Duration<double> >([=](const Nav &nav) {
      return nav.time() - offset;
    });
  }
}

namespace {
  Arrayd toDouble(Array<Angle<double> > X) {
    return X.map<double>([&](Angle<double> x) {
      return x.degrees();
    });
  }

  Arrayd toDouble(Array<Velocity<double> > X) {
    return X.map<double>([&](Velocity<double> x) {
      return x.knots();
    });
  }

  Array<Angle<double> > toAngles(Arrayd X) {
    return X.map<Angle<double> >([&](double x) {
      return Angle<double>::degrees(x);
    });
  }

  Array<Velocity<double> > toVelocities(Arrayd X) {
    return X.map<Velocity<double> >([&](double x) {
      return Velocity<double>::knots(x);
    });
  }

  UniformSamples<Angle<double> > toAngles(UniformSamplesd X) {
    return UniformSamples<Angle<double> >(X.sampling(), toAngles(X.samples()));
  }

  UniformSamples<Velocity<double> > toVelocities(UniformSamplesd X) {
    return UniformSamples<Velocity<double> >(X.sampling(), toVelocities(X.samples()));
  }

  template <typename T>
  void makeDebugPlot(std::string title, Arrayd rawTimesSeconds, Array<T> rawValues,
      UniformSamples<T> filtered, std::string yLabel, FilteredNavData::DebugPlotMode mode) {
      GnuplotExtra plot;
      plot.set_title(title);
      plot.set_style("lines");
      plot.set_xlabel("Time (seconds)");
      plot.set_ylabel(yLabel);

      if (mode == FilteredNavData::SIGNAL) {
        plot.plot_xy(rawTimesSeconds, toDouble(rawValues), "Raw signal");
      }
      Arrayd X = filtered.makeCenteredX();
      if (mode == FilteredNavData::SIGNAL) {
        Arrayd Y = toDouble(filtered.interpolateLinear(X));
        plot.plot_xy(X, Y, "Filtered signal");
      } else {
        Arrayd Y = toDouble(filtered.interpolateLinearDerivative(X));
        plot.plot_xy(X, Y, "Filtered signal derivative");
      }

      plot.show();
  }
}

UniformSamplesd tryFilter(GeneralizedTV tv, Arrayd X, Arrayd Y, double spacing,
    int order, double lambda) {
  if (X.size() == Y.size()) {
    return tv.filter(X, Y, spacing, order, lambda);
  }
  return UniformSamplesd();
}

FilteredNavData::FilteredNavData(Array<Nav> navs, double lambda,
  FilteredNavData::DebugPlotMode mode) {
  if (navs.hasData()) {
    std::sort(navs.begin(), navs.end());
    CHECK(sameBoat(navs));

      ENTERSCOPE("FilteredNavData");
      SCOPEDMESSAGE(INFO, stringFormat("Number of navs: %d", navs.size()));
      _timeOffset = navs[0].time();
      Array<Duration<double> > times = getTimes(navs, _timeOffset);
      Arrayd timesSeconds = times.map<double>(
          [&](Duration<double> t) {
            return t.seconds();
          });
      SCOPEDMESSAGE(INFO, "Get the raw data");
      Array<Angle<double> > magHdg = cleanContinuousAngles(getMagHdg(navs));
      SCOPEDMESSAGE(INFO, "Done mag hdg");
      Array<Angle<double> > awa = cleanContinuousAngles(getAwa(navs));
      SCOPEDMESSAGE(INFO, "Done awa");
      Array<Velocity<double> > aws = getAws(navs);
      SCOPEDMESSAGE(INFO, "Done aws");
      Array<Velocity<double> > watSpeed = getWatSpeed(navs);
      SCOPEDMESSAGE(INFO, "Done wat speed");
      Array<Angle<double> > gpsBearing = cleanContinuousAngles(getGpsBearing(navs));
      SCOPEDMESSAGE(INFO, "Done gps bearing");
      Array<Velocity<double> > gpsSpeed = getGpsSpeed(navs);
      SCOPEDMESSAGE(INFO, "Done gps speed");

      /*
       * order=2 means that we will have
       * a continuous filtered signal
       * with piecewise constant derivatives.
       */
      const int order = 2;

      /*
       * Spacing in seconds between the samples
       * representing the filtered signal
       */
      double spacing = 1.0;

      //UniformSamples<Angle<double> > _awa, _magHdg, _gpsBearing;
      //UniformSamples<Velocity<double> > _watSpeed, _gpsSpeed, _aws;
      GeneralizedTV tv;
      SCOPEDMESSAGE(INFO, "TV filter");
      SCOPEDMESSAGE(INFO, "AWA");
      _awa = toAngles(tryFilter(tv, timesSeconds, toDouble(awa), spacing,
          order, lambda));
      SCOPEDMESSAGE(INFO, "Mag hdg");
      _magHdg = toAngles(tryFilter(tv, timesSeconds, toDouble(magHdg),
          spacing, order, lambda));
      SCOPEDMESSAGE(INFO, "GPS bearing");
      _gpsBearing = toAngles(tryFilter(tv, timesSeconds, toDouble(gpsBearing),
          spacing, order, lambda));
      SCOPEDMESSAGE(INFO, "Wat speed");
      _watSpeed = toVelocities(tryFilter(tv, timesSeconds, toDouble(watSpeed),
          spacing, order, lambda));
      SCOPEDMESSAGE(INFO, "GPS speed");
      _gpsSpeed = toVelocities(tryFilter(tv, timesSeconds, toDouble(gpsSpeed),
          spacing, order, lambda));
      SCOPEDMESSAGE(INFO, "AWS speed");
      _aws = toVelocities(tryFilter(tv, timesSeconds, toDouble(aws),
          spacing, order, lambda));


      assert(_awa.sameSamplingAs(_magHdg));
      assert(_awa.sameSamplingAs(_gpsBearing));
      assert(_awa.sameSamplingAs(_watSpeed));
      assert(_awa.sameSamplingAs(_gpsSpeed));
      assert(_awa.sameSamplingAs(_aws));

      SCOPEDMESSAGE(INFO, "Done TV filter.");
       if (mode != NONE) {
         makeDebugPlot("AWA", timesSeconds, awa,
               _awa, "Angle (degrees)", mode);
         makeDebugPlot("Magnetic heading", timesSeconds, magHdg,
               _magHdg, "Angle (degrees)", mode);
         makeDebugPlot("GPS Bearing", timesSeconds, gpsBearing,
               _gpsBearing, "Angle (degrees)", mode);
         makeDebugPlot("Water speed", timesSeconds, watSpeed,
               _watSpeed, "Speed (knots)", mode);
         makeDebugPlot("GPS speed", timesSeconds, gpsSpeed,
               _gpsSpeed, "Speed (knots)", mode);
         makeDebugPlot("AWS speed", timesSeconds, aws,
               _aws, "Speed (knots)", mode);
       }
  } else {
    LOG(WARNING) << "No navs";
  }
}

FilteredNavData::FilteredNavData(
      TimeStamp timeOffset,
      LineKM sampling,
      Array<Angle<double> > awaSamples,
      Array<Angle<double> > magHdgSamples,
      Array<Angle<double> > gpsBearingSamples,
      Array<Velocity<double> > watSpeedSamples,
      Array<Velocity<double> > gpsSpeedSamples,
      Array<Velocity<double> > awsSamples) :
      _awa(sampling, awaSamples),
      _magHdg(sampling, magHdgSamples),
      _gpsBearing(sampling, gpsBearingSamples),
      _watSpeed(sampling, watSpeedSamples),
      _gpsSpeed(sampling, gpsSpeedSamples),
      _aws(sampling, awsSamples)
      {
  int count = awaSamples.size();
  assert(count == magHdgSamples.size());
  assert(count == gpsBearingSamples.size());
  assert(count == watSpeedSamples.size());
  assert(count == gpsSpeedSamples.size());
  assert(count == awsSamples.size());
}

Arrayd FilteredNavData::makeCenteredX() const {
  return _awa.makeCenteredX();
}

HorizontalMotion<double> FilteredNavData::gpsMotion(double localTime) const {
  return HorizontalMotion<double>::polar(_gpsSpeed.interpolateLinear(localTime),
      _gpsBearing.interpolateLinear(localTime));
}

HorizontalMotion<double> FilteredNavData::gpsMotionAtIndex(int index) const {
  return HorizontalMotion<double>::polar(_gpsSpeed.get(index),
      _gpsBearing.get(index));
}

FilteredNavData::NoiseStdDev FilteredNavData::estimateNoise(Array<Nav> navs) const {
  MeanAndVar awa;
  MeanAndVar magHdg;
  MeanAndVar gpsBearing;
  MeanAndVar watSpeed;
  MeanAndVar gpsSpeed;
  MeanAndVar aws;
  for (int i = 0; i < navs.size(); i++) {
    auto n = navs[i];
    auto to = n.time();
    double t = (to - _timeOffset).seconds();
    awa.add((n.awa() - _awa.interpolateLinear(t)).normalizedAt0().degrees());
    magHdg.add((n.magHdg() - _magHdg.interpolateLinear(t)).normalizedAt0().degrees());
    gpsBearing.add((n.gpsBearing() - _gpsBearing.interpolateLinear(t)).normalizedAt0().degrees());
    watSpeed.add((n.watSpeed() - _watSpeed.interpolateLinear(t)).knots());
    gpsSpeed.add((n.gpsSpeed() - _gpsSpeed.interpolateLinear(t)).knots());
    aws.add((n.aws() - _aws.interpolateLinear(t)).knots());
  }
  return FilteredNavData::NoiseStdDev(Angle<double>::degrees(awa.standardDeviation()),
      Angle<double>::degrees(magHdg.standardDeviation()),
      Angle<double>::degrees(gpsBearing.standardDeviation()),
      Velocity<double>::knots(watSpeed.standardDeviation()),
      Velocity<double>::knots(gpsSpeed.standardDeviation()),
      Velocity<double>::knots(aws.standardDeviation()));
}


Array<Duration<double> > FilteredNavData::timesSinceOffset() const {
  int n = size();
  Array<Duration<double> > durs(n);
  for (int i = 0; i < n; i++) {
    durs[i] = Duration<double>::seconds(indexToX()(i));
  }
  return durs;
}

Array<FilteredNavData::Indexed> FilteredNavData::makeIndexedInstrumentAbstractions() const {
  return Spani(0, size()).map<Indexed>([&](int i) {return makeIndexedInstrumentAbstraction(i);});
}


}
