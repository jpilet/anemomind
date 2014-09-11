/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#ifndef FILTEREDPOLARPOINTS_H_
#define FILTEREDPOLARPOINTS_H_

#include <server/common/MDArray.h>
#include <device/Arduino/libraries/PhysicalQuantity/PhysicalQuantity.h>
#include <server/common/Json.h>
#include <server/nautical/polar/PolarPoint.h>
#include <server/common/Span.h>
#include <server/plot/extra.h>

namespace sail {

class FilteredPolarPoints {
 public:
  FilteredPolarPoints() {}
  FilteredPolarPoints(Velocity<double> step_, MDArray2i inds_);


  MDArray2i inds() const {
    return _inds;
  }

  Velocity<double> step() const {
    return _step;
  }

  class Point {
   public:
    Point() {}
    Point(const MDArray2i &slice3x1_,
        Velocity<double> vel_, Spani span_);

    // For descending order
    bool operator< (const Point &other) const {
      return _span.width() > other._span.width();
    }

    Spani span() const {
      return _span;
    }

    const PolarPoint &polarPoint() const {
      return _ppt;
    }

    int xIndex() const {
      return _inds(0, 0);
    }

    int yIndex() const {
      return _inds(1, 0);
    }

    int twsIndex() const {
      return _inds(2, 0);
    }

    Velocity<double> step() const {
      return _step;
    }

    Velocity<double> x() const {
      return double(xIndex())*_step;
    }

    Velocity<double> y() const {
      return double(yIndex())*_step;
    }
   private:
    Velocity<double> _step;
    MDArray2i _inds;
    PolarPoint _ppt;
    Spani _span;
  };

  Velocity<double> toVelocity(int index) const {
    return double(index)*_step;
  }


  Array<Point> getStablePoints() const;

  void plotForWindSpeed(int windSpeed, GnuplotExtra *dst);
 private:
  Velocity<double> _step;
  MDArray2i _inds;
  bool change(int index) const;
};

namespace json {
  Poco::Dynamic::Var serialize(const FilteredPolarPoints &fpp);
  bool deserialize(Poco::Dynamic::Var src, FilteredPolarPoints *dst);
}

}

#endif /* FILTEREDPOLARPOINTS_H_ */
