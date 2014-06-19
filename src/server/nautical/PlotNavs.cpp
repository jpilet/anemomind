/*
 *  Created on: 2014-06-17
 *      Author: Jonas Östlund <uppfinnarjonas@gmail.com>
 */

#include <server/nautical/TestdataNavs.h>
#include <server/nautical/grammars/Grammar001.h>
#include <server/plot/extra.h>
#include <server/common/string.h>
#include <server/nautical/grammars/TreeExplorer.h>
#include <server/common/ArrayBuilder.h>


using namespace sail;

namespace {
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

   class ExternalTwsValueExtract : public ValueExtract {
    public:
     const char *name() {return "externalTws";}
     double extract(const Nav &x) {return x.externalTws().knots();}
   };

   class ExternalTwaValueExtract : public ValueExtract {
    public:
     const char *name() {return "externalTwa";}
     double extract(const Nav &x) {
       return x.externalTwa().normalizedAt0().degrees();
     }
   };

    class LeewayValueExtract : public ValueExtract {
     public:
      const char *name() {return "leeway";}
      double extract(const Nav &x) {return (x.magHdg() - x.gpsBearing()).normalizedAt0().degrees();}
    };

    class GpsSpeedValueExtract : public ValueExtract {
     public:
      const char *name() {return "gpsSpeed";}
      double extract(const Nav &x) {return x.gpsSpeed().knots();}
    };

    class TimeValueExtract : public ValueExtract {
     public:
      const char *name() {return "time";}
      double extract(const Nav &x) {
        int64_t t = x.time().toMilliSecondsSince1970() / int64_t(1000);
        return double(t);
      }
    };

    typedef std::map<std::string, ValueExtract*> ValueExtractMap;

    template <class ValueExtractType>
    void registerValueExtract(ValueExtractMap *dst) {
      static ValueExtractType x;
      (*dst)[std::string(x.name())] = &x;
    }

   class WatSpeedValueExtract : public ValueExtract {
    public:
     const char *name() {return "watSpeed";}
     double extract(const Nav &x) {return x.watSpeed().knots();}
   };

   ValueExtractMap makeValueExtractMap() {
     ValueExtractMap dst;
     registerValueExtract<AwaValueExtract>(&dst);
     registerValueExtract<AwsValueExtract>(&dst);
     registerValueExtract<LeewayValueExtract>(&dst);
     registerValueExtract<GpsSpeedValueExtract>(&dst);
     registerValueExtract<WatSpeedValueExtract>(&dst);
     registerValueExtract<TimeValueExtract>(&dst);
     registerValueExtract<ExternalTwsValueExtract>(&dst);
     registerValueExtract<ExternalTwaValueExtract>(&dst);
     return dst;
   }

   ValueExtractMap &getTheValueExtractMap() {
     static ValueExtractMap theMap = makeValueExtractMap();
     return theMap;
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

   Array<std::string> extractPlottablesOrDefault(int argc, const char **argv) {
     ValueExtractMap &map = getTheValueExtractMap();
     ArrayBuilder<std::string> toPlot(3);
     for (int i = 1; i < argc; i++) {
       const char *s = argv[i];
       if (map.find(s) != map.end()) {
         toPlot.add(s);
         if (toPlot.size() == 3) {
           std::cout << "The plot can not display more than 3 dimensions. Omitting " << s << std::endl;
         }
       }
     }
     if (toPlot.size() < 2) {
       if (toPlot.size() == 0) {
         std::cout << "No values to plot provided. Plotting default values." << std::endl;
       } else {
         std::cout << "Too few values to plot. Plotting default values instead." << std::endl;
       }
       return Array<std::string>::args("awa", "aws", "leeway");
     }

     return toPlot.get();
   }

   void extractValues(std::string tag, Array<Nav> navs, MDArray2d dst) {
    ValueExtractMap &map = getTheValueExtractMap();
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
    plot.set_xlabel(labels[0]);
    plot.set_ylabel(labels[1]);
    if (toPlotCount >= 3) {
      plot.set_zlabel(labels[2]);
    }
    char axis[] = { 'x', 'y', 'z' };
    for (int i = 0; i < toPlotCount; ++i) {
      if (labels[i] == "time") {
        plot.cmd(stringFormat("set %cdata time\n", axis[i]));
        plot.cmd("set timefmt \"%s\"\n");
        plot.cmd(stringFormat("set format %c \"%s\"\n",
                              axis[i], "%m/%d/%Y %H:%M:%S"));
        plot.cmd("set xtics nomirror rotate by -45\n");
      }
    }
    plot.plot(plotData);
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
    std::string whatToPlot;
    for (pair<std::string, ValueExtract*> name : getTheValueExtractMap()) {
      whatToPlot += " " + name.first;
    }

    std::cout << "Nav plotter. Command line options:\n"
    "  --slice <from-index> <to-index> : select subrange of navs to plot\n"
    "  --navpath <path> : select custom data path with nmea data to load\n"
    "  --select-node : compute a parse tree and select a node in that parse\n"
    "  tree for which to make the plot\n" << whatToPlot << "\n";

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


