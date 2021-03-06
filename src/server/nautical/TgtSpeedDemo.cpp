/*
 *  Created on: 2014-08-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/logimport/TestdataNavs.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/Nav.h>
#include <server/common/ArrayIO.h>
#include <server/common/string.h>
#include <server/nautical/TargetSpeed.h>
#include <algorithm>
#include <server/transducers/Transducer.h>

namespace {
  using namespace sail;
  using namespace sail::NavCompat;

  bool isUpwindAwa(Angle<double> angle) {
    double x = cos(angle);
    return cos(Angle<double>::degrees(45)) <= x && x <= cos(Angle<double>::degrees(20));
  }

  Arrayb getUpwind(NavDataset navs) {
    return transduce(Range(navs), trMap([=](const Nav &x) {
      return isUpwindAwa(x.awa());
    }), IntoArray<bool>());
  }

  bool isDownwindAwa(Angle<double> angle) {
    double x = cos(angle);
    return x <= 0;
  }

  Arrayb getDownwind(NavDataset navs) {
    return transduce(Range(navs), trMap([=](const Nav &x) {
      return isDownwindAwa(x.awa());
    }), IntoArray<bool>());
  }

  Array<Velocity<double> > getTws(NavDataset navs) {
    return transduce(Range(navs), trMap([=](const Nav &x) {
      return x.externalTws();
    }), IntoArray<Velocity<double>>());
  }

  Arrayd makeQuantiles() {
    return Arrayd{0.9, 0.75, 0.50};
  }

  Array<Velocity<double> > makeBounds() {
    return Array<Velocity<double> >::fill(25, [=](int i) {return Velocity<double>::knots(i);});
  }


  void makePlot(bool isUpwind, Array<Velocity<double> > tws, Array<Velocity<double > > vmg,
      Arrayb upwind, Arrayb downwind) {
    Arrayd quantiles = makeQuantiles();
    Array<Velocity<double> > bounds = makeBounds();

    Arrayb sel = (isUpwind? upwind : downwind);

    bool refimpl = true;
    if (refimpl) {
      TargetSpeed tgt(isUpwind, tws.slice(sel), vmg.slice(sel), bounds, quantiles);
      std::cout << EXPR_AND_VAL_AS_STRING(double(countTrue(upwind))/upwind.size()) << std::endl;
      tgt.plot();
    }
  }

  void protoAlgoOnTestdata(int argc, const char **argv) {
    ArgMap amap;
    registerGetTestdataNavs(amap);
    if (amap.parse(argc, argv) != ArgMap::Error) {
      NavDataset data = getTestdataNavs(amap);
      Array<Velocity<double> > tws = getTws(data);
      Arrayb upwind = getUpwind(data);
      Arrayb downwind = getDownwind(data);
      makePlot(true, tws, calcExternalVmg(data, true), upwind, downwind);
      makePlot(false, tws, calcExternalVmg(data, false), upwind, downwind);
    }
  }
}

int main(int argc, const char **argv) {
  using namespace sail;

  protoAlgoOnTestdata(argc, argv);

  return 0;
}
