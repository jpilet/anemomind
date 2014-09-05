/*
 *  Created on: 2014-09-04
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/nautical/TestdataNavs.h>
#include <server/plot/extra.h>
#include <device/Arduino/libraries/TrueWindEstimator/TrueWindEstimator.h>
#include <algorithm>

using namespace sail;

namespace {
  HorizontalMotion<double> calcTW(const Nav &x) {
    double params[TrueWindEstimator::NUM_PARAMS];
    TrueWindEstimator::initializeParameters(params);
    return TrueWindEstimator::computeTrueWind(params, x);
  }

  class PlottableNav {
   public:
    PlottableNav() {}
    PlottableNav(const Nav &x) : _nav(x), _tw(calcTW(x)) {
      _twa = calcTwa(_tw, x.gpsBearing());
    }
    double x() const {
      return _nav.gpsSpeed().knots()*sin(_twa);
    }

    double y() const {
      return _nav.gpsSpeed().knots()*cos(_twa);
    }

    double z() const {
      return calcTws(_tw).knots();
    }

    bool operator< (const PlottableNav &other) const {
      return z() < other.z();
    }
   private:
    Nav _nav;
    HorizontalMotion<double> _tw;
    Angle<double> _twa;
  };

  Array<PlottableNav> makePlottable(Array<Nav> navs) {
    return navs.map<PlottableNav>([&](const Nav &x) {return PlottableNav(x);});
  }

  Array<PlottableNav> reduceFromTheSides(Array<PlottableNav> navs, Velocity<double> tws, int count) {
    std::sort(navs.begin(), navs.end());
    int removeCount = navs.size() - count;
    double Z = tws.knots();
    for (int i = 0; i < removeCount; i++) {
      if (std::abs(Z - navs.first().z()) < std::abs(Z - navs.last().z())) {
        navs = navs.sliceBut(1);
      } else {
        navs = navs.sliceFrom(1);
      }
    }
    return navs;
  }

  Arrayd getX(Array<PlottableNav> navs) {
    return navs.map<double>([&](const PlottableNav &n) {return n.x();});
  }

  Arrayd getY(Array<PlottableNav> navs) {
    return navs.map<double>([&](const PlottableNav &n) {return n.y();});
  }

  Arrayd getZ(Array<PlottableNav> navs) {
    return navs.map<double>([&](const PlottableNav &n) {return n.z();});
  }

  void makePolar2dScatter(Array<Nav> navs0, Velocity<double> tws, int count) {
    Array<PlottableNav> navs = reduceFromTheSides(makePlottable(navs0), tws, count);
    GnuplotExtra plot;
    plot.set_style("points");
    plot.plot_xy(getX(navs), getY(navs));
    plot.show();
  }

  void makePolar3dScatter(Array<Nav> navs0) {
    Array<PlottableNav> navs = makePlottable(navs0);
    GnuplotExtra plot;
    plot.set_style("points");
    plot.plot_xyz(getX(navs), getY(navs), getZ(navs));
    plot.show();
  }
}

int main(int argc, const char **argv) {
  ArgMap amap;
  registerGetTestdataNavs(amap);
  amap.registerOption("--slice2d", "For a given windspeed in knots (first argument), "
      "plot the N (second argument) closest point to that wind speed.").setArgCount(2).setUnique();
  amap.setHelpInfo("Produces a scatter polar plot, no calibration.");
  if (amap.parseAndHelp(argc, argv)) {
    Array<Nav> navs = getTestdataNavs(amap);
    if (amap.optionProvided("--slice2d")) {
      Array<ArgMap::Arg*> args = amap.optionArgs("--slice2d");
      double twsKnots = args[0]->parseDoubleOrDie("First argument of --slice2d");
      int count = args[1]->parseIntOrDie("Second argument of --slice2d");
      if (count <= 0) {
        std::cout << "It doesn't make sense to plot a non-positive number of points" << std::endl;
        return -1;
      }
      if (navs.size() < count) {
        std::cout << "Not enough navs, reducing the number of points to plot." << std::endl;
        count = navs.size();
      }
      makePolar2dScatter(navs, Velocity<double>::knots(twsKnots), count);
    } else {
      makePolar3dScatter(navs);
    }
    return 0;
  }
  return -1;
}


