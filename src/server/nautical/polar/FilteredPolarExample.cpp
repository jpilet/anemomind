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
#include <device/Arduino/libraries/TargetSpeed/PolarSpeedTable.h>



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

  void outputOptParams(std::string optOutFilename, Arrayd params) {
    std::ofstream file(optOutFilename);
    for (int i = 0; i < params.size(); i++) {
      file << params[i] << std::endl;
    }
  }
}

int main(int argc, const char **argv) {
  double lambda = 16.0;
  double stepSizeKnots = 0.5;
  int maxIter = 300;

  int tableTwsCount = 24;
  int tableTwaCount = 12;

  double bandwidthKnots = 1.0;

  int levelCtrlCount = 4;

  int chunkSize = 10000;
  std::string outFilename;
  std::string outOptFilename = "optimized_polar.txt";
  std::string outOptFilenameTemp = "optimized_polar_temp.txt";
  std::string optLoadFilename = "optimized_polar_temp.txt";
  std::string curveMatFilename  = "curvematrix.txt";

  ArgMap amap;
  registerGetTestdataNavs(amap);

  // Commands related to the initial filtering
  amap.registerOption("--reg", "Set the regularization parameter").setArgCount(1).store(&lambda);
  amap.registerOption("--step", "Set the step size in the quantization").setArgCount(1).store(&stepSizeKnots);
  amap.registerOption("--plot-x", "Plot speed along x-axis in polar plot");
  amap.registerOption("--plot-y", "Plot speed along y-axis in polar plot");
  amap.registerOption("--plot-tws", "Plot true wind speed");
  amap.registerOption("--chunk-size", "Set the chunk size").store(&chunkSize);
  amap.registerOption("--max-iter", "Set the maximum number of filtering iterations").setArgCount(1).store(&maxIter);

  // General command to save result
  amap.registerOption("--save", "Provide a filename to save the result").setArgCount(1).store(&outFilename);

  // Commands to look at the filtered result
  amap.registerOption("--view-spans", "Load a file [filename] and analyze the [n] longest stable spans").setArgCount(2);
  amap.registerOption("--interactive-slice", "Only with --view-spans: Slice the quantized values for a particular wind speed");
  amap.registerOption("--optimize", "Optimize the loaded data using [n] control points, to a max speed of [m] knots").setArgCount(2);
  amap.registerOption("--load-opt", "Load file with optimized parameters").setArgCount(1).store(&optLoadFilename);
  amap.registerOption("--save-curve-mat", "Save the optimized curves").setArgCount(1).store(&curveMatFilename);

  amap.registerOption("--build-table", "Writes a table of the optimized polar to [filename]").setArgCount(1);
  amap.registerOption("--table-tws-count", "Set the tws count of the table").setArgCount(1).store(&tableTwsCount);
  amap.registerOption("--table-twa-count", "Set the twa count of the table").setArgCount(1).store(&tableTwaCount);


  amap.setHelpInfo(" Example usage: \n"
      "View the filtered spans:     ./nautical_polar_FilteredPolarExample --view-spans filtered.json 1000 --save filtered.txt --interactive-slice\n"
      "Optimize:                    ./nautical_polar_FilteredPolarExample --view-spans filtered.json 3000 --optimize 7 20\n"
      "Visualize optimized results: ./nautical_polar_FilteredPolarExample --view-spans filtered.json 3000 --optimize 7 20 --load-opt optimized_polar_temp.txt --save-curve-mat curvematrix.txt\n"
      "\n"
      "Another example, with initial filtering:\n"
      "./nautical_polar_FilteredPolarExample --navpath ~/Documents/anemomind/anemomind/datasets/psaros33_Banque_Sturdza/ --save psaros33.filtered.json\n"
      "./nautical_polar_FilteredPolarExample --view-spans psaros33.filtered.json 400 --optimize 8 24 --build-table polartable.dat"
      );

  if (amap.parseAndHelp(argc, argv)) {
    Velocity<double> stepSize = Velocity<double>::knots(stepSizeKnots);
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
          PolarSurfaceParam param(cparam, maxTws, twsLevelCount, levelCtrlCount);

          Array<PolarPoint> subpts = pts.sliceTo(count).map<PolarPoint>([&](const FilteredPolarPoints::Point &x) {return x.polarPoint();});
          Velocity<double> bandwidth = Velocity<double>::knots(bandwidthKnots);
          PolarDensity density(bandwidth, subpts, true);
          LevmarSettings settings;
          settings.verbosity = 2;
          settings.setDrawf(param.paramCount(), [=](Arrayd datai) {
            outputOptParams(outOptFilenameTemp, datai);
            std::cout << "Saved intermediate results to " << outOptFilenameTemp << std::endl;
            //param.plot(datai);
          });
          std::cout << "Number of parameters to optimize: " << param.paramCount() << std::endl;

          Arrayd params;
          if (amap.optionProvided("--load-opt")) {
            params = loadMatrixText<double>(optLoadFilename).getStorage();
            std::cout << EXPR_AND_VAL_AS_STRING(params) << std::endl;
          } else {
            params = optimizePolar(param, density, param.generateSurfacePoints(600), Arrayd(), settings);
            outputOptParams(outOptFilename, params);
          }

          if (!curveMatFilename.empty()) {
            MDArray2d mat = param.makeVertexData(params);
            std::cout << EXPR_AND_VAL_AS_STRING(mat.rows()) << std::endl;
            std::cout << EXPR_AND_VAL_AS_STRING(mat.cols()) << std::endl;
            saveMatrix(curveMatFilename, mat, 5, 15);
          }

          if (amap.optionProvided("--build-table")) {
            Arrayd vertices(param.vertexDim());
            param.paramToVertices(params, vertices);

            auto args = amap.optionArgs("--build-table");
            std::string tableName = args[0]->value();
            Velocity<double> marg = Velocity<double>::knots(1.0e-6);
            Velocity<double> twsStep = (1.0/tableTwsCount)*(maxTws - marg);


            class SpeedLookUp {
             public:
              SpeedLookUp(PolarSurfaceParam &p, Arrayd &v) : _param(p), _vertices(v) {}
              Velocity<double> operator() (Velocity<double> tws, Angle<double> twa) {
                return _param.targetSpeed(_vertices, tws, twa);
              };
             private:
              PolarSurfaceParam &_param;
              Arrayd &_vertices;
            };

            std::ofstream file(tableName);
            PolarSpeedTable::build(twsStep,
                tableTwsCount, tableTwaCount,
                SpeedLookUp(param, vertices),
                &file);
          }

          GnuplotExtra plot;
          param.plot(params, &plot);
          plot.show();
        }
        return 0;
      } else {
        LOG(FATAL) << "Failed to load file";
        return -1;
      }
    } else {
      Array<Nav> navs = getTestdataNavs(amap);
      Array<PolarPoint> pts = navsToPolarPoints(navs).slice([&](const PolarPoint &x) {return !x.hasNaN();});
      MDArray2d data = makeDataMatrix(pts);

      Array<LineKM> maps = makeVelMaps(stepSize);
      MDArray2i inds = quantFilterChunked(maps, data, lambda, chunkSize, maxIter);
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
