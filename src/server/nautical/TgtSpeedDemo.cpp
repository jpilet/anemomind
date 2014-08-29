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



  class RefImplTgtSpeed {
   public:
    RefImplTgtSpeed(bool isUpwind_, Array<Velocity<double> > tws, Array<Velocity<double> > vmg,
            Array<Velocity<double> > bounds, Arrayd quantiles_);

    Array<Velocity<double> > binCenters;
    Array<Array<Velocity<double> > > medianValues;
    bool isUpwind;
    Arrayd quantiles;

    void plot();
  };

  int lookUp(Array<Velocity<double> > bounds, Velocity<double> tws) {
    int count = bounds.size();
    if (bounds.last() <= tws) {
      return -1;
    }
    for (int i = count-1; i >= 0; i--) {
      if (bounds[i] <= tws) {
        return i;
      }
    }
    return -1;
  }

  Arrayi lookUp(Array<Velocity<double> > bounds, Array<Velocity<double> > tws) {
    int count = tws.size();
    Arrayi bins(count);
    for (int i = 0; i < count; i++) {
      bins[i] = lookUp(bounds, tws[i]);
    }
    return bins;
  }

  Array<Array<Velocity<double> > > groupVmg(bool isUpwind, int binCount, Arrayi bins, Array<Velocity<double> > vmg) {
    Array<ArrayBuilder<Velocity<double> > > builders(binCount);
    int count = vmg.size();
    int sign = (isUpwind? 1 : -1);
    for (int i = 0; i < count; i++) {
      int bin = bins[i];
      if (bin != -1) {
        builders[bin].add(vmg[i].scaled(sign));
      }
    }
    Array<Array<Velocity<double> > > groups = builders.map<Array<Velocity<double> > >([=](ArrayBuilder<Velocity<double> > x) {return x.get();});
    for (int i = 0; i < binCount; i++) {
      std::sort(groups[i].begin(), groups[i].end());
    }
    return groups;
  }

  void outputMedianValues(Array<Velocity<double> > data, Arrayd quantiles, int index, Array<Array<Velocity<double> > > out) {
    int qCount = quantiles.size();
    LineKM map(0, 1.0, 0, data.size() - 1);
    int count = data.size();
    for (int i = 0; i < qCount; i++) {
      int i2 = int(round(map(quantiles[i])));
      if (0 <= i2 && i2 < data.size()) {
        out[i][index] = data[i2];
      } else {
        out[i][index] = Velocity<double>::knots(-1);
      }
    }
  }

  RefImplTgtSpeed::RefImplTgtSpeed(bool isUpwind_, Array<Velocity<double> > tws,
      Array<Velocity<double> > vmg,
      Array<Velocity<double> > bounds, Arrayd quantiles_) {
    isUpwind = isUpwind_;
    quantiles = quantiles_;

    std::cout << EXPR_AND_VAL_AS_STRING(bounds
        .map<double>([=](Velocity<double> x) {return x.knots();})) << std::endl;
    std::cout << EXPR_AND_VAL_AS_STRING(   tws.sliceTo(30)
        .map<double>([=](Velocity<double> x) {return x.knots();})) << std::endl;

    Arrayi bins = lookUp(bounds, tws);
    int binCount = bounds.size() - 1;
    Array<Array<Velocity<double > > > groups = groupVmg(isUpwind, binCount, bins, vmg);
    std::cout << EXPR_AND_VAL_AS_STRING(groups.map<int>([=](Array<Velocity<double> > x) {return x.size();})) << std::endl;

    int qCount = quantiles.size();
    binCenters = Array<Velocity<double> >(binCount);
    medianValues = Array<Array<Velocity<double> > >::fill(qCount, [=](int i) {return Array<Velocity<double> >(binCount);});
    for (int i = 0; i < binCount; i++) {
      binCenters[i] = (bounds[i] + bounds[i + 1]).scaled(0.5);
      outputMedianValues(groups[i], quantiles, i, medianValues);
    }
  }

  void RefImplTgtSpeed::plot() {
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.set_xlabel("TWS (knots)");
    plot.set_ylabel("VMG (knots)");
    plot.set_title((isUpwind? "Upwind" : "Downwind"));
    auto mapper = [](Velocity<double> x) {return x.knots();};
    for (int i = 0; i < medianValues.size(); i++) {
      plot.plot_xy(binCenters.map<double>(mapper), medianValues[i].map<double>(mapper), stringFormat("Quantile %.3g", quantiles[i]));
    }
    plot.show();
  }

  void makePlot(bool isUpwind, Array<Velocity<double> > tws, Array<Velocity<double > > vmg,
      Arrayb upwind, Arrayb downwind) {
    Arrayd quantiles = makeQuantiles();
    Array<Velocity<double> > bounds = makeBounds();

    Arrayb sel = (isUpwind? upwind : downwind);

    bool refimpl = false;
    if (refimpl) {
      RefImplTgtSpeed tgt(isUpwind, tws.slice(sel), vmg.slice(sel), bounds, quantiles);
      std::cout << EXPR_AND_VAL_AS_STRING(double(countTrue(upwind))/upwind.size()) << std::endl;
      tgt.plot();
    } else {
      int s = (isUpwind? 1 : -1);
      TargetSpeedData tgt(tws, vmg.map<Velocity<double> >([=](Velocity<double> x) {
        return x.scaled(s);}),
        24, Velocity<double>::knots(0), Velocity<double>::knots(24), quantiles);
      tgt.plot(isUpwind? "Upwind" : "Downwind");
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


