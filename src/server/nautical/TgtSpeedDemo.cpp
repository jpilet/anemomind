/*
 *  Created on: 2014-08-28
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/common/PathBuilder.h>
#include <server/nautical/Nav.h>
#include <server/common/ArrayIO.h>
#include <server/common/ArrayBuilder.h>
#include <server/common/string.h>
#include <server/common/LineKM.h>
#include <server/plot/extra.h>
#include <server/nautical/TargetSpeed.h>
#include <algorithm>

namespace {
  using namespace sail;

  const int yearIndex = 1  - 1;
  const int monthIndex = 2  - 1;
  const int dayOfTheMonthIndex = 3  - 1;
  const int hourIndex = 4  - 1;
  const int minuteIndex = 5  - 1;
  const int secondIndex = 6  - 1;
  const int gpsSpeedIndex = 7  - 1;
  const int awaIndex = 8  - 1;
  const int awsIndex = 9  - 1;
  const int twaIndex = 10  - 1;
  const int twsIndex = 11  - 1;
  const int magHdgIndex = 12  - 1;
  const int watSpeedIndex = 13  - 1;
  const int gpsBearingIndex = 14  - 1;
  const int pos_lat_degIndex = 15  - 1;
  const int pos_lat_minIndex = 16  - 1;
  const int pos_lat_mcIndex = 17  - 1;
  const int pos_lon_degIndex = 18  - 1;
  const int pos_lon_minIndex = 19  - 1;
  const int pos_lon_mcIndex = 20  - 1;
  const int cwdIndex = 21  - 1;
  const int wdIndex = 22  - 1;
  const int daysIndex = 23  - 1;

  bool isUpwindAwa(Angle<double> angle) {
    double x = cos(angle);
    return cos(Angle<double>::degrees(45)) <= x && x <= cos(Angle<double>::degrees(20));
  }

  Arrayb getUpwind(MDArray2d data) {
    int count = data.rows();
    Arrayb ch(count);
    for (int i = 0; i < count; i++) {
      Angle<double> x = Angle<double>::degrees(data(i, awaIndex));
      ch[i] = isUpwindAwa(x);
    }
    return ch;
  }

  Arrayb getUpwind(Array<Nav> navs) {
    return navs.map<bool>([=](const Nav &x) {
      return isUpwindAwa(x.awa());
    });
  }


  bool isDownwindAwa(Angle<double> angle) {
    double x = cos(angle);
    return x <= 0;
  }

  Arrayb getDownwind(MDArray2d data) {
    int count = data.rows();
    Arrayb ch(count);
    for (int i = 0; i < count; i++) {
      Angle<double> x = Angle<double>::degrees(data(i, awaIndex));
      ch[i] = isDownwindAwa(x);
    }
    return ch;
  }

  Arrayb getDownwind(Array<Nav> navs) {
    return navs.map<bool>([=](const Nav &x) {
      return isDownwindAwa(x.awa());
    });
  }


  //vmgGps = data(:, gpsSpeed) .* cos(data(:,twa) * pi / 180);

  Velocity<double> calcVmgGps(Velocity<double> gpsSpeed, Angle<double> twa) {
    double factor = cos(twa);
    return gpsSpeed.scaled(factor);
  }

  Array<Velocity<double> > getVmgGps(MDArray2d data) {
    int count = data.rows();
    Array<Velocity<double> > vmg(count);
    for (int i = 0; i < count; i++) {
      Angle<double> twa = Angle<double>::degrees(data(i, twaIndex));
      Velocity<double> gpsSpeed = Velocity<double>::knots(data(i, gpsSpeedIndex));
      vmg[i] = calcVmgGps(gpsSpeed, twa);
    }
    return vmg;
  }

  Array<Velocity<double> > getVmgGps(Array<Nav> navs) {
    return navs.map<Velocity<double> >([=](const Nav &x) {
      return calcVmgGps(x.gpsSpeed(), x.externalTwa());
    });
  }

  Array<Velocity<double> > getTws(MDArray2d data) {
    int count = data.rows();
    Array<Velocity<double> > tws(count);
    for (int i = 0; i < count; i++) {
      tws[i] = Velocity<double>::knots(data(i, twsIndex));
    }
    return tws;
  }

  Array<Velocity<double> > getTws(Array<Nav> navs) {
    return navs.map<Velocity<double> >([=](const Nav &x) {
      return x.externalTws();
    });
  }

  Arrayd makeQuantiles() {
    return Arrayd::args(0.9, 0.75, 0.50);
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


  void protoAlgoOnSpecialData(bool isUpwind) {
    std::string filename = "/home/jonas/programmering/matlab/irene_tgt_speed/allnavs.txt";
    MDArray2d data = loadMatrixText<double>(filename);

    assert(!data.empty());

    Array<Velocity<double> > vmg = getVmgGps(data);
    Array<Velocity<double> > tws = getTws(data);
    Arrayb upwind = getUpwind(data);
    Arrayb downwind = getDownwind(data);
    makePlot(isUpwind, tws, vmg, upwind, downwind);
  }

  void protoAlgoOnTestdata(int argc, const char **argv) {
    Array<Nav> data = getTestdataNavs(argc, argv);

    Array<Velocity<double> > vmg = getVmgGps(data);
    Array<Velocity<double> > tws = getTws(data);
    Arrayb upwind = getUpwind(data);
    Arrayb downwind = getDownwind(data);
    makePlot(true, tws, vmg, upwind, downwind);
    makePlot(false, tws, vmg, upwind, downwind);
  }
}

int main(int argc, const char **argv) {
  using namespace sail;

  //protoAlgoOnSpecialData(isUpwind);
  protoAlgoOnTestdata(argc, argv);

  return 0;
}


