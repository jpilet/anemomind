/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include "SimpleCalibrator.h"
#include <server/common/ProportionateIndexer.h>
#include <queue>
#include <server/common/ArrayBuilder.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>

namespace sail {

namespace {

  // A class that holds the integrals
  // of
  class NavItg {
   public:
    NavItg(FilteredNavData data);

    // Size of the initial array
    int size() const {
      return awa.size() - 1;
    }

    Integral1d<Angle<double> > awa, magHdg, gpsBearing;
    Integral1d<Velocity<double> > watSpeed, gpsSpeed, aws;
  };

  Angle<double> initAngle = Angle<double>::zero();
  Velocity<double> initSpeed = Velocity<double>::zero();

  template <typename T>
  Integral1d<T> toItg(UniformSamples<T> x) {
    return Integral1d<T>(x.samples(), T::zero());
  }

  NavItg::NavItg(FilteredNavData data) :
    awa(toItg(data.awa())),
    magHdg(toItg(data.magHdg())),
    gpsBearing(toItg(data.gpsBearing())),
    watSpeed(toItg(data.watSpeed())),
    gpsSpeed(toItg(data.gpsSpeed())),
    aws(toItg(data.aws())) {}

  int durationToSampleCount(LineKM sampling, Duration<double> d) {
   double a = sampling.inv(d.seconds());
   double b = sampling.inv(0.0);
   return int(std::abs(a - b));
  }



  class Integrator {
   public:
    Integrator(int width, int gap, NavItg itg) :
      _width(width), _gap(gap), _itg(itg) {}



    // How many indexable locations there are.
    int size() const {
      return _itg.size() - totalWidth() + 1;
    }

    // True size of the data.
    int trueSize() const {
      return _itg.size();
    }

    Angle<double> maneuverStrength(int index) const {
      return evalStep(index, _itg.magHdg);
    }

    Nav leftNav(int index) const {
      return avgNav(index, index + _width);
    }

    Nav rightNav(int index) const {
      int offset = index + _width + _gap;
      return avgNav(offset, offset + _width);
    }

    int totalWidth() const {
      return 2*_width + _gap;
    }
   private:
    Nav avgNav(int from, int to) const {
      Nav dst;
      dst.setAwa(_itg.awa.average(from, to));
      dst.setAws(_itg.aws.average(from, to));
      dst.setGpsBearing(_itg.gpsBearing.average(from, to));
      dst.setGpsSpeed(_itg.gpsSpeed.average(from, to));
      dst.setWatSpeed(_itg.watSpeed.average(from, to));
      dst.setMagHdg(_itg.magHdg.average(from, to));
      return dst;
    }

    template <typename T>
    T evalRight(int pos, Integral1d<T> itg) const {
      int offset = pos + _width + _gap;
      return itg.average(offset, offset + _width);
    }

    template <typename T>
    T evalLeft(int pos, Integral1d<T> itg) const {
      return itg.average(pos, pos + _width);
    }

    template <typename T>
    T evalStep(int pos, Integral1d<T> itg) const {
      return evalRight(pos, itg) - evalLeft(pos, itg);
    }


    int _width, _gap;
    NavItg _itg;
  };

  class RegionMarker {
   public:
    RegionMarker(int size) : _indexer(Arrayd::fill(size, 0.0)) {}

    void mark(int from, int to) {
      // Assign values to the end points
      _indexer.assign(from, 1.0);
      _indexer.assign(to-1, 1.0);
    }

    bool marked(int from, int to) {
      return _indexer.integrate(from, to) > 0.5;
    }
   private:
    ProportionateIndexer _indexer;
  };

  class Loc {
   public:
    Loc(Angle<double> strength, int index) :
      _angle(strength), _index(index) {}

    bool operator< (const Loc &other) const {
      return std::abs(_angle.degrees()) < std::abs(other._angle.degrees());
    }

    int index() const {
      return _index;
    }

    Angle<double> angle() const {
      return _angle;
    }
   private:
    Angle<double> _angle;
    int _index;
  };

  std::ostream &operator<< (std::ostream &s, const Loc &x) {
    s << "Loc(" << x.index() << ", " << x.angle().degrees() << " degrees)";
    return s;
  }

  Array<Loc> findBestLocs(Integrator itg, int count) {
    std::priority_queue<Loc> locs;
    RegionMarker marker(itg.trueSize());
    for (int i = 0; i < itg.size(); i++) {
      auto s = itg.maneuverStrength(i);
      locs.push(Loc(s, i));
    }
    ArrayBuilder<Loc> builder;
    for (int i = 0; i < count; i++) {
      std::cout << EXPR_AND_VAL_AS_STRING(i) << std::endl;
      std::cout << EXPR_AND_VAL_AS_STRING(count) << std::endl;
      if (locs.empty()) {
        break;
      }
      auto loc = locs.top();
      int left = loc.index();
      locs.pop();
      int right = left + itg.totalWidth();
      if (!marker.marked(left, right)) {
        marker.mark(left, right);
        builder.add(loc);
      }
    }
    return builder.get();
  }
}

Corrector<double> SimpleCalibrator::calibrate(FilteredNavData data0) const {
  auto sampling = data0.sampling();
  Integrator itg(durationToSampleCount(sampling, _integrationWidth),
                 durationToSampleCount(sampling, _gap),
                 NavItg(data0));
  auto locs = findBestLocs(itg, _maneuverCount);
  std::cout << EXPR_AND_VAL_AS_STRING(locs) << std::endl;
  return Corrector<double>();
}

}
