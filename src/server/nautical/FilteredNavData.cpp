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
      UniformSamples<T> filtered, std::string yLabel) {
      GnuplotExtra plot;
      plot.set_title(title);
      plot.set_style("lines");
      plot.plot_xy(rawTimesSeconds, toDouble(rawValues), "Raw signal");
      Arrayd X = filtered.makeCentredX();
      plot.plot_xy(X, toDouble(filtered.interpolateLinear(X)), "Filtered signal");
      plot.set_xlabel("Time (seconds)");
      plot.set_ylabel(yLabel);
      plot.show();
  }
}

FilteredNavData::FilteredNavData(Array<Nav> navs, double lambda, bool debug) {
  if (navs.hasData()) {
    std::sort(navs.begin(), navs.end());
    if (sameBoat(navs)) {
      _timeOffset = navs[0].time();
      Array<Duration<double> > times = getTimes(navs, _timeOffset);
      Arrayd timesSeconds = times.map<double>(
          [&](Duration<double> t) {
            return t.seconds();
          });
      Array<Angle<double> > magHdg = cleanContinuousAngles(getMagHdg(navs));
      Array<Angle<double> > awa = cleanContinuousAngles(getAwa(navs));
      Array<Velocity<double> > aws = getAws(navs);
      Array<Velocity<double> > watSpeed = getWatSpeed(navs);
      Array<Angle<double> > gpsBearing = getGpsBearing(navs);
      Array<Velocity<double> > gpsSpeed = getGpsSpeed(navs);

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
      _awa = toAngles(tv.filter(timesSeconds, toDouble(awa), spacing,
          order, lambda));
      _magHdg = toAngles(tv.filter(timesSeconds, toDouble(magHdg),
          spacing, order, lambda));
      _gpsBearing = toAngles(tv.filter(timesSeconds, toDouble(gpsBearing),
          spacing, order, lambda));
      _watSpeed = toVelocities(tv.filter(timesSeconds, toDouble(watSpeed),
          spacing, order, lambda));
      _gpsSpeed = toVelocities(tv.filter(timesSeconds, toDouble(gpsSpeed),
          spacing, order, lambda));
      _aws = toVelocities(tv.filter(timesSeconds, toDouble(aws),
          spacing, order, lambda));
       if (debug) {
         makeDebugPlot("AWA", timesSeconds, awa,
               _awa, "Angle (degrees)");
         makeDebugPlot("Magnetic heading", timesSeconds, magHdg,
               _magHdg, "Angle (degrees)");
         makeDebugPlot("GPS Bearing", timesSeconds, gpsBearing,
               _gpsBearing, "Angle (degrees)");
         makeDebugPlot("Water speed", timesSeconds, watSpeed,
               _watSpeed, "Speed (knots)");
         makeDebugPlot("GPS speed", timesSeconds, gpsSpeed,
               _gpsSpeed, "Speed (knots)");
         makeDebugPlot("AWS speed", timesSeconds, aws,
               _aws, "Speed (knots)");
       }
    } else {
      LOG(WARNING) << "Mixed boat ids";
    }
  } else {
    LOG(WARNING) << "No navs";
  }
}

}
