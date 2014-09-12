/*
 *  Created on: 2014-09-10
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/common/ArgMap.h>
#include <server/plot/extra.h>
#include <server/nautical/TestdataNavs.h>
#include <server/nautical/polar/PolarPointConv.h>
#include <server/math/hmm/QuantFilter.h>
#include <server/math/PolarCoordinates.h>
#include <Poco/JSON/Stringifier.h>
#include <server/nautical/polar/FilteredPolarPoints.h>
#include <server/common/JsonIO.h>
#include <server/common/ArrayIO.h>
#include <server/nautical/polar/PolarOptimizer.h>



using namespace sail;

namespace {
  MDArray2d makeDataMatrix(Array<PolarPoint> pts) {
    int count = pts.size();
    MDArray2d dst(3, count);
    for (int i = 0; i < count; i++) {
      PolarPoint &pt = pts[i];
      dst(0, i) = calcPolarX(true, pt.boatSpeed(), pt.twa()).knots();
      dst(1, i) = calcPolarY(true, pt.boatSpeed(), pt.twa()).knots();
      dst(2, i) = pt.tws().knots();
    }
    return dst;
  }

  Array<LineKM> makeVelMaps(Velocity<double> step) {
    return Array<LineKM>::fill(3, LineKM(0, 1, 0, step.knots()));
  }

  void makeRegPlot(MDArray2d data, MDArray2i inds, double stepSizeKnots, const char *label,
      Array<Duration<double> > t) {
    assert(data.rows() == 1);
    assert(inds.rows() == 1);
    int count = data.cols();
    assert(count == inds.cols());
    Arrayd X(count);
    Arrayd Yraw(count);
    Arrayd Yfiltered(count);
    for (int i = 0; i < count; i++) {
      X[i] = t[i].seconds();
      Yraw[i] = data(0, i);
      Yfiltered[i] = stepSizeKnots*inds[i];
    }
    GnuplotExtra plot;
    plot.set_style("lines");
    plot.set_ylabel(label);
    plot.plot_xy(X, Yraw);
    plot.plot_xy(X, Yfiltered);
    plot.show();
  }

  Array<Duration<double> > calcT(Array<PolarPoint> pts, Array<Nav> navs) {
    TimeStamp ts = navs[pts.first().navIndex()].time();
    int count = pts.size();
    Array<Duration<double> > T(count);
    for (int i = 0; i < count; i++) {
      T[i] = navs[pts[i].navIndex()].time() - ts;
    }
    return T;
  }

  void saveAsMatrix(std::string filename, Array<FilteredPolarPoints::Point> pts) {
    std::ofstream file(filename);
    file << "% Col 1: polar x point (knots)\n"
            "% Col 2: polar y point (knots)\n"
            "% Col 3: tws point (knots)";
    file << "% Col 4: polar x point (index)\n"
            "% Col 5: polar y point (index)\n"
            "% Col 6: tws point (index)\n";
    int count = pts.size();
    for (int i = 0; i < count; i++) {
      FilteredPolarPoints::Point &ppt = pts[i];
      const PolarPoint &pt = ppt.polarPoint();
      file << pt.x().knots() << " " << pt.y().knots() << " " << pt.tws().knots() <<
          " " << ppt.xIndex() << " " << ppt.yIndex() << " " << ppt.twsIndex() << std::endl;
    }
  }

  void listWindSpeeds(std::map<int, int> &m, Velocity<double> step) {
    typedef std::map<int, int>::iterator I;
    for (I i = m.begin(); i != m.end(); i++) {
      int index = i->first;
      std::cout << "Wind speed " << index << " (" << double(index)*step.knots() << " knots): " << i->second << " samples." << std::endl;
    }
  }


  MDArray2d getXY(Array<FilteredPolarPoints::Point> pts) {
    int count = pts.size();
    MDArray2d data(count, 2);
    for (int i = 0; i < count; i++) {
      FilteredPolarPoints::Point &pt = pts[i];
      data(i, 0) = pt.x().knots();
      data(i, 1) = pt.y().knots();
    }
    return data;
  }

  void displayPlot(Array<FilteredPolarPoints::Point> pts) {
    GnuplotExtra plot;
    plot.set_style("points");
    plot.set_xlabel("X [knots]");
    plot.set_xlabel("Y [knots]");
    plot.plot(getXY(pts));
    plot.show();
  }

  void displayPlot(Array<FilteredPolarPoints::Point> pts, int wsIndex) {
    displayPlot(pts.slice([&](const FilteredPolarPoints::Point &pt) {
      return wsIndex == pt.twsIndex();
    }));
  }

  void interactiveSlice(Array<FilteredPolarPoints::Point> pts) {
    Velocity<double> step = pts[0].step();
    std::map<int, int> m;
    for (auto p : pts) {
      m[p.twsIndex()]++;
    }

    bool found = true;
    do {
      listWindSpeeds(m, step);
      std::cout << "Which wind speed? ";
      int index = 0;
      std::cin >> index;
      found = m.find(index) != m.end();
      if (found) {
        displayPlot(pts, index);
      }
    } while (found);
  }
}

int main(int argc, const char **argv) {
  double lambda = 16.0;
  double stepSizeKnots = 0.5;
  int chunkSize = 10000;
  std::string outFilename;

  ArgMap amap;
  registerGetTestdataNavs(amap);

  // Commands related to the initial filtering
  amap.registerOption("--reg", "Set the regularization parameter").setArgCount(1).store(&lambda);
  amap.registerOption("--step", "Set the step size in the quantization").setArgCount(1).store(&stepSizeKnots);
  amap.registerOption("--plot-x", "Plot speed along x-axis in polar plot");
  amap.registerOption("--plot-y", "Plot speed along y-axis in polar plot");
  amap.registerOption("--plot-tws", "Plot true wind speed");
  amap.registerOption("--chunk-size", "Set the chunk size").store(&chunkSize);

  // General command to save result
  amap.registerOption("--save", "Provide a filename to save the result").setArgCount(1).store(&outFilename);

  // Commands to look at the filtered result
  amap.registerOption("--view-spans", "Load a file [filename] and analyze the [n] longest stable spans").setArgCount(2);
  amap.registerOption("--interactive-slice", "Only with --view-spans: Slice the quantized values for a particular wind speed");
  amap.registerOption("--optimize", "Optimize the loaded data using [n] control points, to a max speed of [m] knots").setArgCount(2);

  amap.setHelpInfo(" Example usage: ./nautical_polar_FilteredPolarExample --view-spans filtered.json 1000 --save filtered.txt --interactive-slice");

  if (amap.parseAndHelp(argc, argv)) {
    if (amap.optionProvided("--view-spans")) {
      Array<ArgMap::Arg*> args = amap.optionArgs("--view-spans");
      FilteredPolarPoints fpp;
      if (json::deserialize(json::load(args[0]->value()), &fpp)) {
        int offs = 300;
        std::cout << EXPR_AND_VAL_AS_STRING(fpp.inds().sliceCols(offs, offs + 20)) << std::endl;
        Array<FilteredPolarPoints::Point> pts = fpp.getStablePoints();
        int count = std::min(args[1]->parseIntOrDie(), pts.size());
        for (int i = 0; i < count; i++) {
          FilteredPolarPoints::Point pt = pts[i];
          std::cout << "Span from " << pt.span().minv() << " to " << pt.span().maxv() << " of length " << pt.span().width() << std::endl;
        }
        std::cout << "Total number of stable points: " << pts.size() << std::endl;
        if (!outFilename.empty()) {
          saveAsMatrix(outFilename, pts);
        }
        if (amap.optionProvided("--interactive-slice")) {
          interactiveSlice(pts.sliceTo(count));
        }
        if (amap.optionProvided("--optimize")) {
          Array<ArgMap::Arg*> optargs = amap.optionArgs("--optimize");
          int ctrlCount = optargs[0]->parseIntOrDie();
          Velocity<double> maxTws = Velocity<double>::knots(optargs[1]->parseDoubleOrDie());

          PolarCurveParam cparam(15, ctrlCount, true);
          int twsLevelCount = int(ceil(maxTws.knots()/stepSizeKnots));
          PolarSurfaceParam param(cparam, maxTws, twsLevelCount);

          Array<PolarPoint> subpts = pts.sliceTo(count).map<PolarPoint>([&](const FilteredPolarPoints::Point &x) {return x.polarPoint();});
        }
        return 0;
      } else {
        LOG(FATAL) << "Failed to load file";
        return -1;
      }
    } else {
      Velocity<double> stepSize = Velocity<double>::knots(stepSizeKnots);
      Array<Nav> navs = getTestdataNavs(amap);
      Array<PolarPoint> pts = navsToPolarPoints(navs).slice([&](const PolarPoint &x) {return !x.hasNaN();});
      MDArray2d data = makeDataMatrix(pts);

      Array<LineKM> maps = makeVelMaps(stepSize);
      MDArray2i inds = quantFilterChunked(maps, data, lambda, chunkSize);
      Array<Duration<double> > t = calcT(pts, navs);

      if (amap.optionProvided("--plot-x")) {
        const int i = 0;
        makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "X [knots]", t);
      }
      if (amap.optionProvided("--plot-y")) {
        const int i = 1;
        makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "Y [knots]", t);
      }
      if (amap.optionProvided("--plot-tws")) {
        const int i = 2;
        makeRegPlot(data.sliceRow(i), inds.sliceRow(i), stepSizeKnots, "TWS [knots]", t);
      }
      if (!outFilename.empty()) {
        std::ofstream file(outFilename);
        Poco::Dynamic::Var obj = json::serialize(FilteredPolarPoints(stepSize, inds));

        Poco::JSON::Stringifier::stringify(obj, file, 0, 0);
      }
    }

    return 0;
  }
  return -1;
}
