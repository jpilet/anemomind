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

  Arrayb getUpwind(MDArray2d data) {
    int count = data.rows();
    Arrayb ch(count);
    for (int i = 0; i < count; i++) {
      double x = cos(Angle<double>::degrees(data(i, awaIndex)));
      ch[i] = cos(Angle<double>::degrees(45)) <= x && x <= cos(Angle<double>::degrees(20));
    }
    return ch;
  }

  Arrayb getDownwind(MDArray2d data) {
    int count = data.rows();
    Arrayb ch(count);
    for (int i = 0; i < count; i++) {
      double x = cos(Angle<double>::degrees(data(i, awaIndex)));
      ch[i] = x <= 0;
    }
    return ch;
  }


  //vmgGps = data(:, gpsSpeed) .* cos(data(:,twa) * pi / 180);

  Array<Velocity<double> > getVmgGps(MDArray2d data) {
    int count = data.rows();
    Array<Velocity<double> > vmg(count);
    for (int i = 0; i < count; i++) {
      double factor = cos(Angle<double>::degrees(data(i, twaIndex)));
      vmg[i] = Velocity<double>::knots(data(i, gpsSpeedIndex)).scaled(factor);
    }
    return vmg;
  }

  Array<Velocity<double> > getTws(MDArray2d data) {
    int count = data.rows();
    Array<Velocity<double> > tws(count);
    for (int i = 0; i < count; i++) {
      tws[i] = Velocity<double>::knots(data(i, twsIndex));
    }
    return tws;
  }

  Arrayd makeQuantiles() {
    return Arrayd::args(0.9, 0.75, 0.50);
  }

  Array<Velocity<double> > makeBounds() {
    return Array<Velocity<double> >::fill(25, [=](int i) {return Velocity<double>::knots(i);});
  }



  class RefImplTgtSpeed {
   public:
    RefImplTgtSpeed(bool isUpwind, Array<Velocity<double> > tws, Array<Velocity<double> > vmg,
            Array<Velocity<double> > bounds, Arrayd quantiles);

    Array<Velocity<double> > binCenters;
    Array<Array<Velocity<double> > > medianValues;

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
      out[i][index] = data[int(round(map(quantiles[i])))];
    }
  }

  RefImplTgtSpeed::RefImplTgtSpeed(bool isUpwind, Array<Velocity<double> > tws,
      Array<Velocity<double> > vmg,
      Array<Velocity<double> > bounds, Arrayd quantiles) {

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
    auto mapper = [](Velocity<double> x) {return x.knots();};
    for (int i = 0; i < medianValues.size(); i++) {
      plot.plot_xy(binCenters.map<double>(mapper), medianValues[i].map<double>(mapper));
    }
    plot.show();
  }
}


int main(int argc, const char **argv) {
  using namespace sail;

  std::string filename = "/home/jonas/programmering/matlab/irene_tgt_speed/allnavs.txt";

  MDArray2d data = loadMatrixText<double>(filename);
  assert(!data.empty());

  Array<Velocity<double> > vmg = getVmgGps(data);
  Array<Velocity<double> > tws = getTws(data);
  Arrayb upwind = getUpwind(data);
  Arrayb downwind = getDownwind(data);

  Arrayd quantiles = makeQuantiles();

  Array<Velocity<double> > bounds = makeBounds();

  bool isUpwind = false;

  Arrayb sel = (isUpwind? upwind : downwind);

  RefImplTgtSpeed tgt(isUpwind, tws.slice(sel), vmg.slice(sel), bounds, quantiles);

  std::cout << EXPR_AND_VAL_AS_STRING(double(countTrue(upwind))/upwind.size()) << std::endl;

  tgt.plot();

  return 0;
}


