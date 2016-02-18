/*
 *  Created on: 2015
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/FlowErrors.h>
#include <server/common/Functional.h>

namespace sail {

namespace {
  MeanAndVar evaluateMeanAndVar(
      Array<HorizontalMotion<double> > trueMotion,
      Array<HorizontalMotion<double> > estimatedMotion,
      std::function<double(HorizontalMotion<double>,
          HorizontalMotion<double>)> errorFun) {
    if (estimatedMotion.empty() || trueMotion.empty()) {
      return MeanAndVar();
    } else {
      assert(trueMotion.size() == estimatedMotion.size());
      int count = trueMotion.size();
      MeanAndVar acc;
      for (int i = 0; i < count; i++) {
        double error = errorFun(trueMotion[i], estimatedMotion[i]);
        assert(std::isfinite(error));
        acc.add(error);
      }
      return acc.normalize();
    }
  }
}

FlowErrors::FlowErrors(Array<HorizontalMotion<double> > trueMotion,
          Array<HorizontalMotion<double> > estimatedMotion) {
  _normError = Error<Velocity<double> >(evaluateMeanAndVar(trueMotion, estimatedMotion,
      [=](HorizontalMotion<double> a, HorizontalMotion<double> b) {
      return HorizontalMotion<double>(a - b).norm().knots();
  }), Velocity<double>::knots(1.0));
  _angleError = Error<Angle<double> >(evaluateMeanAndVar(trueMotion, estimatedMotion,
        [=](HorizontalMotion<double> a, HorizontalMotion<double> b) {
        return std::abs((a.angle() - b.angle()).normalizedAt0().degrees());
    }), Angle<double>::degrees(1.0));
  _magnitudeError = Error<Velocity<double> >(evaluateMeanAndVar(trueMotion, estimatedMotion,
        [=](HorizontalMotion<double> a, HorizontalMotion<double> b) {
        return std::abs((a.norm() - b.norm()).knots());
    }), Velocity<double>::knots(1.0));
}

std::ostream &operator<< (std::ostream &s, const FlowErrors &e) {
  s << "FlowError( norm: " << e.norm() << " angle: " << e.angle() << " magnitude: " << e.magnitude() << ")";
  return s;
}

std::ostream &operator<< (std::ostream &s, const WindCurrentErrors &e) {
  s << "Wind and current errors over " << e.count << " samples:" << std::endl;
  s << "  Wind: " << e.wind << std::endl;
  s << "  Current: " << e.current << std::endl;
  return s;
}

WindCurrentErrors compareCorrectors(
    const CorrectorFunction &a, const CorrectorFunction &b,
    NavCollection navs) {
  auto aNavs = a(navs);
  auto bNavs = b(navs);
  auto getWind = [&](const CalibratedNav<double> &x) {return x.trueWindOverGround();};
  auto getCurrent = [&](const CalibratedNav<double> &x) {return x.trueCurrentOverGround();};
  return WindCurrentErrors{
    FlowErrors(toArray(map(aNavs, getWind)),
               toArray(map(bNavs, getWind))),
    FlowErrors(toArray(map(aNavs, getCurrent)),
               toArray(map(bNavs, getCurrent))),
    navs.size()
  };
}


} /* namespace mmm */
