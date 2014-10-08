/*
 *  Created on: 2014-
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/polar/FilteredPolarPoints.h>
#include <server/common/MDArrayJson.h>
#include <server/common/PhysicalQuantityJson.h>
#include <server/common/JsonObjDeserializer.h>
#include <server/common/Json.impl.h>
#include <server/common/ArrayBuilder.h>
#include <algorithm>

namespace sail {


FilteredPolarPoints::FilteredPolarPoints(Velocity<double> step_,
    MDArray2i inds_) :
  _step(step_), _inds(inds_) {
}

FilteredPolarPoints::Point::Point(const MDArray2i &slice3x1_,
    Velocity<double> vel_, Spani span_) : _span(span_), _inds(slice3x1_), _step(vel_) {
  assert(slice3x1_.rows() == 3);
  assert(slice3x1_.cols() == 1);
  double xknots = vel_.knots()*slice3x1_(0, 0);
  double yknots = vel_.knots()*slice3x1_(1, 0);
  double twsknots = vel_.knots()*slice3x1_(2, 0);
  double angle = atan2(xknots, yknots); // obs: should not be atan2(yknots, xknots), because we have nautical polar coordinates.
  _ppt = PolarPoint(Velocity<double>::knots(twsknots), Angle<double>::radians(angle),
      Velocity<double>::knots(sqrt(xknots*xknots + yknots*yknots)));
}

bool FilteredPolarPoints::change(int index) const {
  if (index == 0 || index == _inds.cols()) {
    return true;
  }
  for (int i = 0; i < 3; i++) {
    if (_inds(i, index-1) != _inds(i, index)) {
      return true;
    }
  }
  return false;
}

Array<FilteredPolarPoints::Point> FilteredPolarPoints::getStablePoints() const {
  ArrayBuilder<Point> builder;
  int from = 0;
  for (int i = 0; i < _inds.cols(); i++) {
    int to = i+1;
    if (change(to)) {
      builder.add(Point(_inds.sliceCol(from),
          _step, Spani(from, to)));
      from = to;
    }
  }
  assert(from == _inds.cols());
  Array<Point> pts = builder.get();
  std::sort(pts.begin(), pts.end());
  return pts;
}


namespace json {
  Poco::Dynamic::Var serialize(const FilteredPolarPoints &fpp) {
    Poco::JSON::Object::Ptr obj(new Poco::JSON::Object());
    obj->set("inds", json::serialize(fpp.inds()));
    obj->set("step", json::serialize(fpp.step()));
    return Poco::Dynamic::Var(obj);
  }

  bool deserialize(Poco::Dynamic::Var src, FilteredPolarPoints *dst) {
    MDArray2i inds;
    Velocity<double> step;
    json::ObjDeserializer deser(src);
    deser.get("inds", &inds);
    deser.get("step", &step);
    if (deser.success()) {
      *dst = FilteredPolarPoints(step, inds);
      return true;
    } else {
      *dst = FilteredPolarPoints();
      return false;
    }
  }

}


}
