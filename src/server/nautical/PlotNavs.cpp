/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/grammars/Grammar001.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/nautical/grammars/TreeExplorer.h>


using namespace sail;

namespace {
  double unwrap(Angle<double> x) {
    return x.degrees();
  }

  double unwrap(Velocity<double> x) {
    return x.knots();
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


  class ValueExtract {
   public:
    virtual const char *name() = 0;
    virtual double extract(const Nav &x) = 0;
    virtual ~ValueExtract() {}
  };


  class AwaValueExtract : public ValueExtract {
   public:
    const char *name() {return "awa";}
   double extract(const Nav &x) {return x.awa().normalizedAt0().degrees();}
  };


   class AwsValueExtract : public ValueExtract {
    public:
     const char *name() {return "aws";}
     double extract(const Nav &x) {return x.aws().knots();}
   };

    class LeewayValueExtract : public ValueExtract {
     public:
      const char *name() {return "leeway";}
      double extract(const Nav &x) {return (x.magHdg() - x.gpsBearing()).normalizedAt0().degrees();}
    };

    class TimeValueExtract : public ValueExtract {
     public:
      TimeValueExtract() : _reftime(TimeStamp::UTC(2014, 06, 18, 15, 8, 00)) {}
      const char *name() {return "time";}
      double extract(const Nav &x) {return (x.time() - _reftime).seconds();}
     private:
      TimeStamp _reftime;
    };

    class GpsSpeedValueExtract : public ValueExtract {
     public:
      const char *name() {return "gps-speed";}
      double extract(const Nav &x) {return x.gpsSpeed().knots();}
    };

    typedef std::map<std::string, ValueExtract*> ValueExtractMap;

    void registerValueExtract(ValueExtractMap *dst, ValueExtract *x) {
      (*dst)[std::string(x->name())] = x;
    }

   class WatSpeedValueExtract : public ValueExtract {
    public:
     const char *name() {return "wat-speed";}
     double extract(const Nav &x) {return x.watSpeed().knots();}
   };

   ValueExtractMap makeValueExtractMap() {
     static AwaValueExtract a;
     static AwsValueExtract b;
     static LeewayValueExtract c;
     static TimeValueExtract d;
     static GpsSpeedValueExtract e;
     static WatSpeedValueExtract f;
     ValueExtractMap dst;
     registerValueExtract(&dst, &a);
     registerValueExtract(&dst, &b);
     registerValueExtract(&dst, &c);
     registerValueExtract(&dst, &d);
     registerValueExtract(&dst, &e);
     registerValueExtract(&dst, &f);
     return dst;
   }

   void transferValues(ValueExtract *ve,
       Array<Nav> navs,
       MDArray2d dst) {
     assert(dst.cols() == 1);
     assert(dst.rows() == navs.size());
     int count = navs.size();
     for (int i = 0; i < count; i++) {
       dst(i, 0) = ve->extract(navs[i]);
     }
   }

   void extractValues(std::string tag, Array<Nav> navs, MDArray2d dst) {
    static ValueExtractMap map = makeValueExtractMap();
    if (map.find(tag) == map.end()) {
      dst.setAll(0.0);
      std::cout << "Unknown tag: " << tag << std::endl;
    } else {
      transferValues(map[tag], navs, dst);
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

  void getBoundsByNode(int argc, const char **argv, Array<Nav> navs, int *fromOut, int *toOut) {
    std::string tag("--select-node");
    for (int i = 0; i < argc; i++) {
      if (argv[i] == tag) {
        Grammar001Settings settings;
        Grammar001 g(settings);
        std::shared_ptr<HTree> tree = g.parse(navs);

        std::shared_ptr<HTree> sel = exploreTree(g.nodeInfo(), tree);

        if (bool(sel)) {
          *fromOut = sel->left();
          *toOut = sel->right();
        }
      }
    }
  }

  void dispInfo() {
    std::cout << "Nav plotter. Command line options:\n";
                 "  awa : include apparent wind angle in the plot\n"
                 "  aws : include apparent wind speed in the plot\n"
    "  wat-speed : include water speed in the plot\n"
    "  gps-speed : include gps speed in the plot\n"
    "  leeway : include leeway angle in the plot\n"
    "  time : include time in the plot\n"
    "  --slice <from-index> <to-index> : select subrange of navs to plot\n"
    "  --navpath <path> : select custom data path with nmea data to load\n"
    "  --select-node : compute a parse tree and select a node in that parse tree for which to make the plot\n";
  }
}

int main(int argc, const char **argv) {
  dispInfo();
  Array<std::string> toPlot = extractPlottablesOrDefault(argc, argv);

  std::cout << "Loading test data..." << std::endl;
  Array<Nav> allnavs = getTestdataNavs(argc, argv);
  std::cout << "Done loading " << allnavs.size() << " navs." << std::endl;

  int from = 0;
  int to = allnavs.size();
  getBounds(argc, argv, &from, &to);

  // Override previously sliced portion
  getBoundsByNode(argc, argv, allnavs, &from, &to);


  std::cout << "Use navs from " << from << " to " << to << " of " << allnavs.size() << " navs to make the plot\n";
  makePlot(allnavs.slice(from, to), toPlot);

  return 0;
}


