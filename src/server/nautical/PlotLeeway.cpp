/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/grammars/Grammar001.h>
#include <server/plot/extra.h>
#include <server/common/string.h>


using namespace sail;

namespace {
  double unwrap(Angle<double> x) {
    return x.degrees();
  }

  double unwrap(Velocity<double> x) {
    return x.knots();
  }

  MDArray2d getDataToPlot(Array<Nav> navs) {
    int counter = 0;
    int count = navs.size();
    MDArray2d data(count, 3);
    for (int i = 0; i < count; i++) {
      Nav &n = navs[i];

      data(counter, 0) = unwrap(n.aws()); // What about putting the boat speed (gpsSpeed or watSpeed) here in some other experiment?
      data(counter, 1) = unwrap(n.awa().normalizedAt0());
      data(counter, 2) = unwrap((n.magHdg() - n.gpsBearing()).normalizedAt0());

      counter++;
    }
    return data.sliceRowsTo(counter);
  }


  Array<std::string> getPlottableValues() {
    const int count = 6;
    static std::string v[count] = {"awa", "aws", "leeway", "gps-speed", "time", "wat-speed"};
    return Array<std::string>(count, v);
  }

  bool isPlottable(const char *arg, const Array<std::string> &plottables) {
    for (auto p : plottables) {
      if (arg == p) {
        return true;
      }
    }
    return false;
  }

  Array<std::string> extractPlottables(int argc, const char **argv) {
    Array<std::string> plottables = getPlottableValues();
    int count = plottables.size();
    Array<std::string> extracted(count);
    int counter = 0;
    for (int i = 1; i < argc; i++) {
      const char *arg = argv[i];
      if (isPlottable(arg, plottables)) {
        extracted[counter] = arg;
        counter++;
      }
    }
    return extracted.sliceTo(counter);
  }

  Array<std::string> extractPlottablesOrDefault(int argc, const char **argv) {
    Array<std::string> extracted = extractPlottables(argc, argv);

    const int ndef = 3;
    static std::string def[ndef] = {"awa", "aws", "leeway"};
    Array<std::string> defaultPlottables(ndef, def);

    if (extracted.empty()) {
      return defaultPlottables;
    } else if (extracted.size() != 2 && extracted.size() != 3) {
      std::cout << "Can only make a 2d or 3d scatter plot. Please provide 2 or 3 values to plot, not " << extracted.size() << ":\n";
      for (auto e : extracted) {
        std::cout << "  " << e << std::endl;
      }
      return defaultPlottables;
    }
    return extracted;
  }

  void getBounds(int argc, const char **argv, int *fromOut, int *toOut) {
    std::string tag = "--slice";
    for (int i = 1; i < argc-2; i++) {
      if (tag == argv[i]) {
        *fromOut = atoi(argv[i+1]);
        *toOut = atoi(argv[i+2]);
        std::cout << "Slice from " << *fromOut << " to " << *toOut << std::endl;
        return;
      }
    }
  }

  void transferValues(std::function<double(const Nav &)> f,
      Array<Nav> navs,
      MDArray2d dst) {
    assert(dst.cols() == 1);
    assert(dst.rows() == navs.size());
    int count = navs.size();
    for (int i = 0; i < count; i++) {
      dst(i, 0) = f(navs[i]);
    }
  }

  #define NAVEXPR(expr) [&] (const Nav &x) {return expr;}
  #define TRANSFER(expr) transferValues(NAVEXPR(expr), navs, dst)

  void extractValues(std::string tag, Array<Nav> navs, MDArray2d dst) {
    if (tag == "awa") {
      TRANSFER(x.awa().degrees());
    } else if (tag == "aws") {
      TRANSFER(x.aws().knots());
    } else if (tag == "leeway") {
      TRANSFER((x.magHdg() - x.gpsBearing()).degrees());
    } else if (tag == "time") {
      TimeStamp reftime = TimeStamp::UTC(2014, 06, 18, 15, 8, 00);
      TRANSFER((x.time() - reftime).seconds());
    } else if (tag == "gps-speed") {
      TRANSFER(x.gpsSpeed().knots());
    } else if (tag == "wat-speed") {
      TRANSFER(x.watSpeed().knots());
    } else {
      dst.setAll(0.0);
      std::cout << "Unknown tag: " << tag << std::endl;
    }
  }

  void makePlot(Array<Nav> navs, Array<std::string> toPlot) {
    int navCount = navs.size();
    int toPlotCount = toPlot.size();
    assert(toPlotCount == 2 || toPlotCount == 3);

    Array<std::string> labels = toPlot;
    MDArray2d plotData(navCount, toPlotCount);
    for (int i = 0; i < toPlotCount; i++) {
      extractValues(toPlot[i], navs, plotData.sliceCol(i));
    }

    GnuplotExtra plot;
    plot.plot(plotData);
    plot.set_xlabel(labels[0]);
    plot.set_ylabel(labels[1]);
    if (toPlotCount >= 3) {
      plot.set_zlabel(labels[2]);
    }
    plot.show();
  }
}

int main(int argc, const char **argv) {
  Array<std::string> toPlot = extractPlottablesOrDefault(argc, argv);

  std::cout << "Loading test data..." << std::endl;
  Array<Nav> allnavs = getTestdataNavs(argc, argv);
  std::cout << "Done loading " << allnavs.size() << " navs." << std::endl;

  int from = 0;
  int to = allnavs.size();
  getBounds(argc, argv, &from, &to);

  makePlot(allnavs.slice(from, to), toPlot);

  return 0;
}


